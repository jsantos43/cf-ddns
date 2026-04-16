import { exec } from 'child_process'
import { writeFileSync, readFileSync } from 'fs'

// Set control variables
let firstVerify = true
let lastIpv4 = null
let INTERFACE = null
let API_TOKEN = null
let RECORD_NAME = null
let RECORD_ID = null
let ZONE_ID = null

// Try to read settings.json
try {
  const data = readFileSync('settings.json', 'utf8')
  const settings = JSON.parse(data)
  API_TOKEN = settings?.API_TOKEN
  RECORD_ID = settings?.RECORD_ID
  RECORD_NAME = settings?.RECORD_NAME
  ZONE_ID = settings?.ZONE_ID
  INTERFACE = settings.INTERFACE || 'eth0'

  if (!API_TOKEN || !RECORD_ID || !ZONE_ID || !INTERFACE || !RECORD_NAME)
    throw Error('Missing settings.json values')
} catch (err) {
  console.error(err)
  throw Error("It was not possible to read settings.json")
}

// Try to read ipv4.json
try {
  const data = readFileSync('ipv4.json', 'utf8')
  const info = JSON.parse(data)
  lastIpv4 = info?.ipv4
} catch (err) {}

// Update on Cloudflare
async function updateCloudflare(ipv4) {
  try {
    const url = `https://api.cloudflare.com/client/v4/zones/${ZONE_ID}/dns_records/${RECORD_ID}`
    const req = {
      method: 'PUT',
      headers: {
        'Authorization': `Bearer ${API_TOKEN}`,
        'Content-Type': 'application/json',
      },
      body: JSON.stringify({
        type: 'A',           // A
        name: RECORD_NAME,
        content: ipv4,
        ttl: 120,
        proxied: false,
      }),
    }
    await fetch(url, req)
    console.log("Update request sent to Cloudflare!")
  } catch (err) {
    console.error("It was not possible to update on Cloudflare!")
  }
}

// Verify IPv4 changed
function verifyIpv4() {
  // Command to read the public-facing IPv4 of the interface
  const command = `ip -4 -o addr show dev ${INTERFACE} scope global | awk '{print $4}' | cut -d/ -f1 | head -n1`

  exec(command, async (err, stdout) => {
    const actualIpv4 = stdout.trim()
    if (actualIpv4 !== lastIpv4 || firstVerify) {
      writeFileSync('ipv4.json', `{"ipv4": "${actualIpv4}"}`, 'utf8')
      await updateCloudflare(actualIpv4)

      firstVerify = false
      lastIpv4 = actualIpv4
    }
  })
}

setInterval(verifyIpv4, 5000)