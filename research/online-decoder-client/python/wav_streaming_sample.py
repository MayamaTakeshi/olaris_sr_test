import json
import sys

import librosa
import numpy as np
import requests
import websocket

import settings


class StreamEncoder(json.JSONEncoder):
    def default(self, obj):
        if isinstance(obj, np.integer):
            return int(obj)
        elif isinstance(obj, np.floating):
            return float(obj)
        elif isinstance(obj, np.ndarray):
            return obj.tolist()
        else:
            return super(StreamEncoder, self).default(obj)

# issue one time token
def issue_one_time_token():
    url = f'https://{settings.API_BASE}/v1/issue_token/'
    headers = {
        'accept': 'text/html',
        'Authorization': f'Bearer {settings.API_KEY}',
        'Content-Type': 'application/json',
    }
    body = {
        'product_name': settings.PRODUCT_NAME,
        'organization_id': settings.ORGANIZATION_ID,
        'user_id': settings.USER_ID
    }
    res = requests.post(url, headers=headers, data=json.dumps(body))
    if res.ok:
        return res.content.decode('utf-8')
    else:
        print(f'{res.status_code}: {res.reason}')

    return None

def main(wav_path, out_path):
    ws = None
    try:
        # 先にwavを読み込んでおく
        wav, _ = librosa.load(wav_path, settings.MODEL_SAMPLING_RATE)
        #print(wav)
        wav = wav*32678
        wav = wav.astype(np.int16)
        #print(wav)

        x = 2730
        to_chunk= lambda wav, x: [wav[i:i+x] for i in range(0, len(wav), x)]
        
        # websocket: open
        access_token = issue_one_time_token()
        if not access_token:
            return
        ws = websocket.create_connection(f'wss://{settings.API_BASE}/ws/')
        msg =  {
            "access_token": access_token,
            "type": 'start',
            "sampling_rate": settings.MODEL_SAMPLING_RATE,
            "product_name": settings.PRODUCT_NAME,
            "organization_id": settings.ORGANIZATION_ID,
            "words": settings.WORDS,
        }
        if settings.MODEL_ALIAS:
            msg['model_alias'] = settings.MODEL_ALIAS
        if settings.MODEL_NAME:
            msg['model_name'] = settings.MODEL_NAME
        if settings.USER_ID:
            msg['user_id'] = settings.USER_ID
        ws.send(json.dumps(msg))
        
        if not ws.connected:
            print('not connected')
            return

        # streaming
        with open(out_path, 'w') as f:
            for chunk in to_chunk(wav, x):
                #print('chunk=', chunk)
                msg = {
                    "type":"streamAudio",
                    "stream":list(chunk)
                }
                #print(json.dumps(msg, cls = StreamEncoder))
                ws.send(json.dumps(msg, cls = StreamEncoder))
                result = ws.recv()
                result = json.loads(result)
                if result['type'] == 'end':
                    f.write(result['result']+ '\n')
                    print(result['result'])

            #final
            ws.send(json.dumps({"type":"final"}))

            while True:
                result = ws.recv()
                if not result:
                    continue
                result = json.loads(result)
                if result['type'] == 'final-end':
                    f.write(result['result']+ '\n')
                    print(result['result'])
                    break
                elif result['type'] == 'final-decoding':
                    break
    
    except Exception as e:
        print(e)
    finally:
        if ws and ws.connected:
            ws.close()

if __name__ == '__main__':
    wav_path = sys.argv[1]
    out_path = sys.argv[2]
    main(wav_path, out_path)
