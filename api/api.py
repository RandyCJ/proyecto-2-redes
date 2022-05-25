from flask import Flask, request, jsonify
import socket
import base64
app = Flask(__name__)

@app.route('/api/dns_resolver', methods=['POST'])
def resolve(dns='8.8.8.8'):
    input_json = request.get_json(force=True)

    encoded_data = input_json["data"]
    decoded_data = base64.b64decode(encoded_data).decode("utf-8") 
    
    bytesToSend = str.encode(decoded_data)
    serverAddressPort   = (dns, 53)
    bufferSize          = 1024

    UDPClientSocket = socket.socket(family=socket.AF_INET, type=socket.SOCK_DGRAM)
    UDPClientSocket.sendto(bytesToSend, serverAddressPort)
    msgFromServer = UDPClientSocket.recvfrom(bufferSize)

    encoded_answer = base64.b64encode(msgFromServer).decode("utf-8") 
    dictToReturn = {'answer': encoded_answer}

    return jsonify(dictToReturn)

if __name__ == '__main__':
    app.run(debug=True, host='0.0.0.0', port=443)