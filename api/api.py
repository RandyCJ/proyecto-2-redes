from flask import Flask, request, jsonify
import socket
import base64
app = Flask(__name__)

@app.route('/api/dns_resolver', methods=['POST'])
def resolve():
    """
    Description: Decodes the data from base64
        sends the information to a remote dns
        encodes the response and returns it.
    Input: None
    Output: A dictionary in json format
    """
    try:
        input_json = request.get_json(force=True)

        encoded_data = input_json["data"]
        remote_dns = input_json["dns"]
        port = input_json["port"]

        decoded_data = base64.b64decode(encoded_data)
        serverAddressPort = (remote_dns, port)
        bufferSize = len(decoded_data)

        UDPClientSocket = socket.socket(family=socket.AF_INET, type=socket.SOCK_DGRAM)
        UDPClientSocket.connect(serverAddressPort)
        UDPClientSocket.sendall(decoded_data)
        msgFromServer = UDPClientSocket.recv(bufferSize)

        encoded_answer = base64.b64encode(msgFromServer)
        dictToReturn = {'answer': encoded_answer.decode()}
    except:
        dictToReturn = {'answer': 'Wrong data sent'}

    return jsonify(dictToReturn)

if __name__ == '__main__':
    app.run(debug=True, host='0.0.0.0', port=443)