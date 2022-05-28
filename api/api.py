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
    decoded_data = base64.b64decode(encoded_data).decode("utf-8") 
    print("decoded_data")
    print(decoded_data)
    print("decoded_data type")
    print(type(decoded_data))
    bytesToSend = str.encode(decoded_data)
    print("bytesToSend")
    print(bytesToSend)
    print("bytesToSend type")
    print(type(bytesToSend))
    serverAddressPort   = ('8.8.8.8', 53)
    print("serverAddressPort")
    print(serverAddressPort)
    bufferSize          = 1024
    print("bufferSize")
    print(bufferSize)

    UDPClientSocket = socket.socket(family=socket.AF_INET, type=socket.SOCK_DGRAM)
    print("UDPClientSocket")
    print(UDPClientSocket)
    UDPClientSocket.sendto(bytesToSend, serverAddressPort)
    print("UDPClientSocket despues del sendto")
    print(UDPClientSocket)
    msgFromServer, _ = UDPClientSocket.recvfrom(bufferSize)
    print("msgFromServer")
    print(msgFromServer)
    msgFromServer = msgFromServer[0]
    print("msgFromServer despues de sacar el indice 0")
    print(msgFromServer)

    encoded_answer = base64.b64encode(msgFromServer).decode("utf-8")
    print("encoded_answer")
    print(encoded_answer)
    dictToReturn = {'answer': encoded_answer}
    print("dictToReturn")
    print(dictToReturn)

    return jsonify(dictToReturn)

if __name__ == '__main__':
    app.run(debug=True, host='172.19.0.3', port=443)