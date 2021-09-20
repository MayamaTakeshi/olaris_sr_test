'use strict'

var fs = require('fs');

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
  if(process.argv.length != 4) {
    console.error("Invalid number of arguments. Required: wav_file max_bytes_per_chunk")
    process.exit(1)
  } 
  const wav_file = process.argv[2]
  const highWaterMark = parseInt(process.argv[3])
  if(highWaterMark < 1024) {
    console.error("max_bytes_per_chunk is invalid. It must be 1024 or higher.")
    process.exit(1)
  }

  const accessToken = await issueToken()
  if (accessToken === null) {
    console.error('Token could not be issued.')
    return
  }

  setInterval(() => {
  }, 5000)

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
    
    fs.writeFileSync(OUTPUT_FILE, '', (err) => {
      if (err) throw err
    })
    ws.onmessage = function (event) {
      //console.log(`ws.onmessage ${JSON.stringify(event.data)}`)
      const res = JSON.parse(event.data)
      //console.log(`ws.onmessage ${res.result}`)
      if (res.type === 'end') {
        fs.appendFile(OUTPUT_FILE, `${res.result}\n`, (err) => {
          if (err) console.error(err)
        })
        console.log(res.result)
      }
    }

    var wav = require('wav');
     
    var file = fs.createReadStream(wav_file,  {'highWaterMark': highWaterMark});
    var reader = new wav.Reader();
     
    // the "format" event gets emitted at the end of the WAVE header
    reader.on('format', function (format) {
       console.log(`Format: ${JSON.stringify(format)}`); 
    });
     
    // pipe the WAVE file to the Reader instance
    ws.on('open', () => {
        console.log('ws open')
        file.pipe(reader);

        reader.on('data', chunk => { 
            //console.log(`got reader data len=${chunk.length} ws.readyState=${ws.readyState}`)
            var l = chunk.length
              var buf = []
              for (var i = 0; i < l / 2; i += 1) {
                buf[i] = chunk.readInt16LE(i*2, true)
                //if (buf[i] > 1.0) buf[i] = 1.0
                //else if (buf[i] < -1.0) buf[i] = -1.0
              }
              var bufferArray =  Array.prototype.slice.call(buf)
              //console.log("length", bufferArray.length)
              var msg = {
                type: 'streamAudio',
                stream: bufferArray
              }
              //console.log(bufferArray)
              var m = JSON.stringify(msg);
              //console.log(m);
              ws.send(m)
          })
          .on('error', err => {
            console.error('reader threw an error:', err)
          })
     })
  } catch (err) {
    console.error(err)
    process.exit()
  }

}

main()
