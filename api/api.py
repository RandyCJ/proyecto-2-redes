from flask import Flask, request, jsonify
import socket
import base64
app = Flask(__name__)

@app.route('/api/dns_resolver', methods=['POST'])
def resolve():
    input_json = request.get_json(force=True)

    encoded_data = input_json["data"]
    print("encoded_data")
    print(encoded_data)
    decoded_data = base64.b64decode(encoded_data)
    print("decoded_data")
    print(decoded_data)
    print("decoded_data type")
    print(type(decoded_data))
    serverAddressPort   = ('8.8.8.8', 53)
    print("serverAddressPort")
    print(serverAddressPort)
    bufferSize          = 1024
    print("bufferSize")
    print(bufferSize)

    UDPClientSocket = socket.socket(family=socket.AF_INET, type=socket.SOCK_DGRAM)
    print("UDPClientSocket")
    print(UDPClientSocket)
    UDPClientSocket.connect(serverAddressPort)
    print("UDPClientSocket despues del connect")
    print(UDPClientSocket)
    UDPClientSocket.sendall(decoded_data)
    print("UDPClientSocket despues del sendall")
    print(UDPClientSocket)
    msgFromServer = UDPClientSocket.recv(bufferSize)
    print("msgFromServer")
    print(msgFromServer)

    encoded_answer = base64.b64encode(msgFromServer)
    print("encoded_answer")
    print(encoded_answer)
    dictToReturn = {'answer': encoded_answer}
    print("dictToReturn")
    print(dictToReturn)

    return jsonify(dictToReturn)

if __name__ == '__main__':
    app.run(debug=True, host='0.0.0.0', port=443)