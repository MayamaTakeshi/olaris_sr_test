'use strict'

const {
  API_BASE,
  MODEL_NAME,
  MODEL_ALIAS,
  SAMPLE_RATE_HERTZ,
  PRODUCT_NAME,
  ORGANIZATION_ID,
  USER_ID,
  API_KEY,
  OUTPUT_FILE,
  WORDS
} = require('./const')

// issue_one_time_token
async function issueToken() {
  const options = {
    method: 'POST',
    uri: `https://${API_BASE}/v1/issue_token/`,
    headers: {
      'accept': 'text/html',
      'Authorization': `Bearer ${API_KEY}`,
      'Content-type': 'application/json'
    },
    body: {
      product_name: PRODUCT_NAME,
      organization_id: ORGANIZATION_ID,
      user_id: USER_ID
    },
    json: true
  }
  let token = null
  const rp = require('request-promise')
  await rp(options)
    .then(res => {
      token = res
    })
    .catch(err => {
      console.error(err)
    })
  return token
}

async function main() {
  const accessToken = await issueToken()
  if (accessToken === null) {
    console.error('Token could not be issued.')
    return
  }

  // websocket: opne
  try {
    const WebSocket = require('ws')
    const ws = new WebSocket(`wss://${API_BASE}/ws/`)
    ws.onopen = function() { 
      let msg = {
        access_token: accessToken,
        type: 'start',
        sampling_rate: SAMPLE_RATE_HERTZ,
        product_name: PRODUCT_NAME,
        organization_id: ORGANIZATION_ID,
        words: WORDS
      }
      if (MODEL_ALIAS) {
        msg.model_alias = MODEL_ALIAS
      }
      if (MODEL_NAME) {
        msg.model_name = MODEL_NAME
      }
      if (USER_ID) {
        msg.user_id = USER_ID
      }
      var m = JSON.stringify(msg);
      //console.log(m);
      ws.send(m);
    }
    
    const fs = require('fs')
    fs.writeFileSync(OUTPUT_FILE, '', (err) => {
      if (err) throw err
    })
    ws.onmessage = function (event) {
      const res = JSON.parse(event.data)
      if (res.type === 'end') {
        fs.appendFile(OUTPUT_FILE, `${res.result}\n`, (err) => {
          if (err) console.error(err)
        })
        console.log(res.result)
      }
    }

    const recorder = require('node-record-lpcm16')
    const recording = recorder
      .record({
        sampleRate: SAMPLE_RATE_HERTZ,
        channels : 1,
        threshold: 0.5, //silence threshold
        recordProgram: 'rec', // Try also "arecord" or "sox"
        silence: '3.0', //seconds of silence before ending
        'Content-Type': 'audio/wav'
      })
    
    console.log('* Start recording (\'ctrl + c\' to stop)')

    // streaming
    recording.stream()
      .on('data', chunk => { var l = chunk.length
        if(ws.readyState === 1) {
          var buf = []
          for (var i = 0; i < l / 2; i += 1) {
            buf[i] = chunk.readInt16LE(i*2, true)
            // if (buf[i] > 1.0) buf[i] = 1.0
            // else if (buf[i] < -1.0) buf[i] = -1.0
          }
          var bufferArray =  Array.prototype.slice.call(buf)
          var msg = {
            type: 'streamAudio',
            stream: bufferArray
          }
          //console.log(bufferArray)
          var m = JSON.stringify(msg);
          //console.log(m);
          ws.send(m)
        }
      })
      .on('error', err => {
        console.error('recorder threw an error:', err)
      })
  } catch (err) {
    console.error(err)
    process.exit()
  }
}

main()
