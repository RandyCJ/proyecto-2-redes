**Instituto Tecnológico de Costa Rica**

**Proyecto 2**

**Redes**

**Número de Grupo: 20**

Elaborado por:

Jeremy Madrigal Portilla - 2019258245

Randall Gabriel Zumbado Huertas - 2019082664

Randy Conejo Juárez - 2019066448

Profesor: Gerardo Nereo Campos Araya

Alajuela, 3 de junio de 2022

## Instrucciones de ejecución

El proyecto fue implementado en un sistema Ubuntu 20.04, por lo que se
recomienda utilizar este sistema para replicar las instrucciones de
forma correcta.

Como primer paso instalamos Docker

`
  $ sudo apt install docker.io
`

Ahora iniciamos Portainer, que es una interfaz gráfica para monitorear
los servicios que creemos con Docker-compose.

  ```
  $ sudo docker run -d -p 8000:8000 -p 9000:9000 \--name=portainer \--restart=always -v /var/run/docker.sock:/var/run/docker.sock portainer/portainer-ce
  ```

Instalamos también docker-compose

`  $ sudo apt install docker-compose
`

Abrimos nuestro navegador web de preferencia e ingresamos a
localhost:9000
![image19](https://user-images.githubusercontent.com/61055501/165863460-bfe00551-c8bd-4d07-a595-28ef80b109fa.png)



Aquí definimos una contraseña, tiene que ser de mínimo 8 caracteres.

Una vez definida ingresamos a Get Started
![image17](https://user-images.githubusercontent.com/61055501/165863502-8a45aa35-37c1-429f-ba34-c1655e7a7785.png)


Aquí seleccionamos local
![image20](https://user-images.githubusercontent.com/61055501/165863529-263b72fa-762b-4337-bdc6-d2aec2b3e78f.png)


Aquí nos saldrá el dashboard, donde podremos ver todos los contenedores
y redes que vayamos levantando.
![image6](https://user-images.githubusercontent.com/61055501/165863543-e679ead8-7061-4c7c-bbfa-f593008bce10.png)


Ahora entramos a la terminal y nos colocamos en la ruta del proyecto,
ingresamos como administrador.

  `
  $ sudo su
  `
  
Una vez ahí ejecutamos el siguiente comando para construir y levantar
los contenedores.

  `
  $ docker-compose build && docker-compose up
  `

Y esperamos que todas las instancias estén levantadas. Para confirmar
que elasticsearch se levantó ingresamos en nuestro navegador a
localhost:9200. Deberíamos ver la siguiente página.

![image19](https://user-images.githubusercontent.com/61055501/171784265-03bc1638-934f-4dba-9684-8b5e527c15bc.png)


Para confirmar que Kibana se levantó, ingresamos a localhost:5601.
Deberíamos ver la siguiente página.

![image9](https://user-images.githubusercontent.com/61055501/171784323-d883c1b2-8735-4727-a901-d1c6486c4988.png)



Una vez hecho esto, creamos un índice zones en elasticsearch, ejecutamos
el siguiente comando

  `
  $ curl -X PUT 10.5.0.5:9200/zones
  `

Deberíamos recibir el siguiente mensaje, que confirma que se creó el
índice.

`{\"acknowledged\":true,\"shards\_acknowledged\":true,\"index\":\"zones\"}`

Una vez creado el índice, nos vamos a Kibana e ingresamos a DevTools.

![image25](https://user-images.githubusercontent.com/61055501/171784345-549e0191-98fc-45d6-a88f-eec74744e87e.png)



Si realizamos un GET zones, debería retornarnos el índice que creamos

![image3](https://user-images.githubusercontent.com/61055501/171784366-65000127-55b8-489c-a1d3-eaf221c22601.png)

Vamos a ver como insertar registros a elasticsearch, puede abrir el archivo `kibana_commands.txt` para copiar y pegar los comandos más facilmente.

Para guardar un hostname en el índice zones, podemos escribir este
comando. 

![image7](https://user-images.githubusercontent.com/61055501/171784401-6e97fdf0-130b-472b-b7f5-c4f9b302f104.png)  
*Esta es la estructura que debemos utilizar para guardar registros que funcionen con nuestro programa, se puede excluir el campo index al crear el registro, 
el programa lo añadirá la primera vez que se busque ese hostname en elasticsearch, si este campo no contiene un número o caracteres que no sean numéricos, el programa no funcionará correctamente.*


Si hacemos un GET de ese registro, nos lo retornará

![image16](https://user-images.githubusercontent.com/61055501/171784429-6e43af69-930d-451e-a58d-9f0e49a63422.png)



![image22](https://user-images.githubusercontent.com/61055501/171784459-9dad157f-aeaf-4bd8-b2a6-027b97eaaed7.png)



Una vez hecho esto, estamos listos para ejecutar el interceptor de DNS,
en la sección de pruebas realizadas veremos cómo actualizar un registro,
esto para cambiar IP, o agregar más de una, y cambiar el campo index
para verificar el funcionamiento de round robin.


Una vez llegados aquí el proyecto se levantó correctamente.

## Pruebas realizadas

***DNS INTERCEPTOR***

Para probar el Interceptor, ingresamos a la terminal a nslookup, e
ingresamos a nuestra ip del dns interceptor.

`
$ nslookup
`  
`
server 10.5.0.7
`

Una vez ahí, comenzamos con las pruebas.

Primero vamos a probar si cuando buscamos un registro que exista en
elasticsearch, el interceptor nos retorna la IP que se encuentre ahí
almacenada. Para ello colocamos www.google.com en la terminal.
Dado que antes guardamos un registro de www.google.com con la ip:
192.168.20.1, debería retornarnos esa.

![image20](https://user-images.githubusercontent.com/61055501/171785347-1d237eab-95f7-4955-ad1c-5b4ec80a6fce.png)


Vemos que es correcto.

Ahora si ingresamos un hostname que no existe en elasticsearch, debería
enviar la solicitud al DNS API, y este retornar la respuesta del
servidor DNS 8.8.8.8. En este caso probaremos con www.twitter.com.

![image29](https://user-images.githubusercontent.com/61055501/171785423-a64f0ebf-2b3e-4200-8e0e-71dcb7d1b0db.png)



Como vemos, el hostname de twitter no existe en elasticsearch, por lo
que se resolvió el hostname desde DNS API. Ahora, si agregamos un
registro para twitter en elasticsearch, debería retornarnos la IP que
ahí se indica.

![image14](https://user-images.githubusercontent.com/61055501/171785456-fe41b0f4-6460-4c6b-95f2-a08541237c8d.png)


![image31](https://user-images.githubusercontent.com/61055501/171785482-3aa0c369-164e-44f9-b290-59f864255c26.png)


![image1](https://user-images.githubusercontent.com/61055501/171785520-aad0cbe8-e580-49e8-b8a4-70ce5535b20a.png)


Ahora vamos a probar round robin. En este caso vamos a actualizar el
registro de google para añadir más IPs.

Para actualizar un registro desde kibana y solo actualizar un campo, lo
hacemos de la siguiente manera.

![image17](https://user-images.githubusercontent.com/61055501/171785554-c7b6af2f-0a6b-4cbd-bb9e-342acf82cdc0.png)



Vemos que si hacemos el GET, solo el campo IP se actualizó

![image24](https://user-images.githubusercontent.com/61055501/171785564-74efd071-9ace-4c5c-8e06-0c8de0a09f5b.png)  
![image15](https://user-images.githubusercontent.com/61055501/171785795-e5ec0b32-6bdc-445e-a8d6-4fca3f757358.png)


Ahora, si hacemos dos peticiones dns en nslookup a www.google.com, el interceptor
nos retornará cada vez un IP diferente, hasta que haya pasado por todos
los IPs disponibles y vuelva al primero. Algo a tener en cuenta con
nuestra implementación es que, por un motivo que desconocemos, cada
request nos retorna dos IPs. Es por eso que en los casos donde hay un
registro en elasticsearch con un solo IP, este se ve dos veces. Pero al
tener más de una IP se van a ver dos IPs de los que pertenecen al
hostname ingresado.

![image11](https://user-images.githubusercontent.com/61055501/171785601-5b6083bf-ed05-4e4e-a7b5-f612e4fff8a0.png)



En la imagen anterior vemos cómo realizamos un total de tres peticiones,
en la primera petición retornó el segundo y tercer IP, en la segunda
petición retornó el cuarto y el primer IP, aquí se cumple el ciclo. Y
vemos que en la tercera petición volvió a retornar el segundo y tercer
IP. Por lo que vemos que round robin está funcionando correctamente.

Ahora, qué pasa si malintencionadamente modificamos el index desde
Kibana para poner un número mayor a la cantidad de IPs disponibles.


![image4](https://user-images.githubusercontent.com/61055501/171785716-781fa342-7d67-4030-bda9-03b7bfcbea06.png)
![image26](https://user-images.githubusercontent.com/61055501/171785734-d3d6cc12-b00c-4d8c-b621-09dcd840ce42.png)


En este caso, lo que realizará el interceptor es retornar el primer IP
de la lista, y colocará el índice en 1. Nuestro interceptor retorna dos
IPs, pero vemos que la primera que retornó es la que está en la posición
0, y de ahí sigue en orden.

![image6](https://user-images.githubusercontent.com/61055501/171785775-465fc4e4-6a38-41e6-afd5-90f52686430b.png)



Y dado que retornó dos IPs, si hacemos un GET vemos que el index se
movió hasta dos, pues sería la siguiente IP en el orden.

![image15](https://user-images.githubusercontent.com/61055501/171785795-e5ec0b32-6bdc-445e-a8d6-4fca3f757358.png)


Ahora, qué pasaría si en el campo de IPs no ponemos ninguna IP, o si
dejamos un dato inválido que no es una IP, como un texto con letras. En
esos casos nuestro Interceptor detectará que en ese campo no existe una
IP, por lo que pasará la request a nuestro DNS API.

![image18](https://user-images.githubusercontent.com/61055501/171785819-8e580697-5cea-4add-b738-1dc91a7a8c05.png)
![image12](https://user-images.githubusercontent.com/61055501/171785837-770d88b5-2f19-4432-83da-8c939bbda684.png)


Ahora ponemos www.google.com, y nos retornará la respuesta obtenida de nuestro DNS API.

![image10](https://user-images.githubusercontent.com/61055501/171785876-89c4134c-a7db-4570-92c7-a0fd87b4d09c.png)

**DNS API**

***Instalando postman***

Esta es una herramienta que nos servirá para probar la API

Abrir la terminal

![image32](https://user-images.githubusercontent.com/61055501/171787737-b8f08cb1-25c8-4330-af98-50ebc572c4cf.png)



Escribir el siguiente comando

`
$ sudo snap install postman
`

Poner la contraseña de su computadora

![image37](https://user-images.githubusercontent.com/61055501/171787750-10eb4ed2-60de-46ec-a0d7-c5a559a72975.png)



De esta manera se descargará postman.  
Abrir postman presionando el ícono

![image42](https://user-images.githubusercontent.com/61055501/171787781-18bf800b-5a23-47ba-a767-9626d11372cb.png)



Una vez abierto presionar el botón blanco con un + arriba a la izquierda, abajo del new anaranjado.

![image34](https://user-images.githubusercontent.com/61055501/171787809-0abec58e-2f03-4452-87c2-8176f42fd584.png)


Esto abrirá una plantilla de request  

Donde dice GET presionamos la pestaña y seleccionamos POST  

Donde dice Enter request URL escribimos lo siguiente  
`
localhost:443/api/dns_resolver
`

Seleccionamos la sección que dice body debajo de los apartados anteriores y luego elegimos raw, la ventana entonces se verá así

![image21](https://user-images.githubusercontent.com/61055501/171787887-b27625a2-5c19-4490-99e1-9552f0e32b66.png)

Para las pruebas simplemente es necesario escribir un json siguiendo
el siguiente formato, las pruebas unitarias realizadas simplemente
se copió y pegó el body del request indicado en la documentación
dentro de esta sección

`{"dns": "DNS remota", "port": 53, "data":"datos en base 64"}`

por ejemplo  

`{"dns": "8.8.8.8", "port": 53, "data":"rOEBAAABAAAAAAAAA3d3dwZnb29nbGUDY29tAAABAAE="}`

Presionar send, al hacer esto recibiremos una response en el cuadro de abajo.

***Casos "happy path"***

Request enviado de Google

`{"dns": "8.8.8.8", "port": 53, "data":
"rOEBAAABAAAAAAAAA3d3dwZnb29nbGUDY29tAAABAAE="}`

Response esperada

`{"answer":
"rOGBgAABAAEAAAAAA3d3dwZnb29nbGUDY29tAAABAAHADAABAAEAAAEfAASs2aXE"}`

Response recibida

`{"answer":
"rOGBgAABAAEAAAAAA3d3dwZnb29nbGUDY29tAAABAAHADAABAAEAAAEfAASs2aXE"}`

Evidencia

![image39](https://user-images.githubusercontent.com/61055501/171788468-65b76eb6-51a4-4ca6-bb18-3753bdd6f82e.png)


Request enviado de Nación

`{"dns": "8.8.8.8", "port": 53, "data":
"1GQBAAABAAAAAAAABWExNjgxBGRzY3IGYWthbWFpA25ldAAAHAAB"}`

Response esperada

`{"answer":
"1GSBgAABAAIAAAAABWExNjgxBGRzY3IGYWthbWFpA25ldAAAHAABwAwAHAABAAAAFAAQJgAUGYQAAAAAAAAAX2UdOsAMABwAAQAAABQAECYAFBmEAAAAAAAAAF9lHVI="}`

Response recibida

`{"answer":
"1GSBgAABAAIAAAAABWExNjgxBGRzY3IGYWthbWFpA25ldAAAHAABwAwAHAABAAAAFAAQJgAUGYQAAAAAAAAAX2UdOsAMABwAAQAAABQAECYAFBmEAAAAAAAAAF9lHVI="}`

Evidencia

![image35](https://user-images.githubusercontent.com/61055501/171788647-33953ea3-a729-49e2-b12f-39642634f958.png)



Request enviado de Facebook

`{"dns": "8.8.8.8", "port": 53, "data":
"SjMBAAABAAAAAAAACXN0YXItbWluaQRjMTByCGZhY2Vib29rA2NvbQAAHAAB"}`

Response esperada

`{"answer":
"SjOBgAABAAEAAAAACXN0YXItbWluaQRjMTByCGZhY2Vib29rA2NvbQAAHAABwAwAHAABAAAAPAAQKgMogPEsAYP6zrAMAAAl3g=="}`

Response recibida

`{"answer":
"SjOBgAABAAEAAAAACXN0YXItbWluaQRjMTByCGZhY2Vib29rA2NvbQAAHAABwAwAHAABAAAAPAAQKgMogPEsAYP6zrAMAAAl3g=="}`

Evidencia

![image36](https://user-images.githubusercontent.com/61055501/171788662-83674c32-86a1-4023-a97d-123061d39767.png)



Request enviado de Twitter

`{"dns": "8.8.8.8", "port": 53, "data":
"T4ABAAABAAAAAAAAB3R3aXR0ZXIDY29tAAAcAAE="}`

Response esperada

`{"answer":
"T4CBgAABAAAAAQAAB3R3aXR0ZXIDY29tAAAcAAHADAAGAAEAAAEeADwDbnMxA3AyNgZkeW5lY3QDbmV0AAp6b25lLWFkbWluBmR5bmRuc8AUd6MViQAADhAAAAJYAAk6gAAAADw="}`

Response recibida

`{"answer":
"T4CBgAABAAAAAQAAB3R3aXR0ZXIDY29tAAAcAAHADAAGAAEAAAEeADwDbnMxA3AyNgZkeW5lY3QDbmV0AAp6b25lLWFkbWluBmR5bmRuc8AUd6MViQAADhAAAAJYAAk6gAAAADw="}`

Evidencia

![image38](https://user-images.githubusercontent.com/61055501/171788697-06c6eed3-57ce-42cc-867d-ec8636ce8924.png)



**Casos negativos**

Request enviado de Google con un carácter menos

`{"dns": "8.8.8.8", "port": 53, "data":
"rOEBAAABAAAAAAAAA3d3dwZnb29nbGUDY29tAAABAAE"}`

Response esperada

`{"answer": "Wrong data sent"}`

Response recibida

`{"answer": "Wrong data sent"}`

Evidencia

![image41](https://user-images.githubusercontent.com/61055501/171788714-80ed32cf-2142-4496-9e80-1ab0b0f4a1cb.png)

Request enviado con caracteres especiales

`{"dns": "8.8.8.8", "port": 53, "data": "°!#$%&()=?¡"}`

Response esperada

`{"answer": "Wrong data sent"}`

Response recibida

`{"answer": "Wrong data sent"}`

Evidencia

![image40](https://user-images.githubusercontent.com/61055501/171788732-574a65cf-49c0-40ed-93b7-f0833f4f28b1.png)


Request enviado con enteros

`{"dns": "8.8.8.8", "port": 53, "data": 1}`

Response esperada

`{"answer": "Wrong data sent"}`

Response recibida

`{"answer": "Wrong data sent"}`

Evidencia

![image33](https://user-images.githubusercontent.com/61055501/171788747-fd94ec64-5085-479b-bfe8-267ef7a01f6b.png)


Request enviado con flotantes

`{"dns": "8.8.8.8", "port": 53, "data": 1.1}`

Response esperada

`{"answer": "Wrong data sent"}`

Response recibida

`{"answer": "Wrong data sent"}`

Evidencia

![image27](https://user-images.githubusercontent.com/61055501/171788769-181befab-38e4-4da6-b092-e7f06504a276.png)


Request enviado sin data

`{"dns": "8.8.8.8", "port": 53, "data": ""}`

Response esperada

`{"answer": "Wrong data sent"}`

Response recibida

`{"answer": "Wrong data sent"}`

Evidencia

![image28](https://user-images.githubusercontent.com/61055501/171788784-040f4882-25b9-4836-958a-da002f579975.png)


Request enviado vacío

`{}`

Response esperada

`{"answer": "Wrong data sent"}`

Response recibida

`{"answer": "Wrong data sent"}`

Evidencia

![image23](https://user-images.githubusercontent.com/61055501/171788802-4d22e9ed-e80c-4a1a-b9d3-02557a2f7f76.png)


## Conclusiones

-   Se puede concluir la importancia de las APIs para comunicar diversos
servicios, utilizar este diseño fue muy beneficioso para la
velocidad de respuesta y también ahorró esfuerzo en la parte de
complejidad por lo que pensamos que fue la aproximación adecuada
al problema presentado.

-   Se puede concluir que postman es una herramienta fundamental para
probar APIs, su uso durante el desarrollo de este proyecto ayudó a
los participantes a identificar distintos errores cometidos
durante el mismo.

-   Se concluyó que existen varias maneras de enviar un response dns
para diferentes tipos de datos que se quieran devolver.

-   Se concluyó que el DNS header se puede almacenar en una estructura
propia de C y no afecta tanto el rendimiento como para causar un
retraso significativo para enviar el response.

-   Se concluyó que el uso de certificados SSL permiten realizar
consultas al API de una forma más segura para enviar paquetes POST
de mejor manera.

-   Se concluyó que la herramienta elasticsearch es muy útil y bastante
potente para la búsqueda y realización de response en archivos
JSON, permite realizar pruebas de una forma sencilla y rápida.

-   Se concluyó que el DNS de google (8.8.8.8) no siempre retorna IPs
que resuelven la página web que se puso en el hostname.

-   Se concluyó la importancia que tiene la construcción de paquetes
DNS, estos mismos nos ayuda a poder conocer correctamente los
bytes que este paquete administra y cómo lo realiza tanto un DNS
como realizarlo desde el inicio con solo una ip de retorno.

-   Se concluyó que el encode de Base64 ayuda permitió en este proyecto
poder enviar bytes correctamente a través de nuestro DNS
interceptor hacia el API que seguidamente esta API nos ayuda a
obtener una respuesta efectiva, por lo tanto el Base64 permite
enviar y recibir bytes guardando su integridad.

-   Se concluyó que realizar pruebas unitarias permiten tanto a la
calidad de código como a poder corregir problemas en cuanto a
diferentes partes de todo el programa, permite probar cada
funcionalidad aparte sin dependencia de las demás partes por lo
cuál ayuda al proceso en general del testing.

## Recomendaciones

-   Se recomienda realizar el proyecto en el sistema operativo Ubuntu
20.04, tomando este como host para docker.

-   Se recomienda utilizar docker-compose para automatizar la creación
de servicios.

-   Es muy recomendado utilizar postman para probar la API incluso antes
de meter esta en un contenedor de docker, de esta manera no se
atrasa con la implementación realizada en C.

-   Se recomienda utilizar Flask para implementar DNS APIs, esto debido
a que es realmente sencillo el uso de dicha biblioteca e incluso
es de fácil instalación en docker.

-   Se recomienda guardar los headers y response DNS en un archivo .txt,
de esta manera luego con herramientas como hexyl y hexdump podemos
ver si están correctos

-   Se recomienda utilizar la imagen de ubuntu 20.04 al implementar el
dns interceptor, ya que se puede instalar herramientas como hexyl
para ver que los headers y responses de dns están correctos.

-   Se recomienda utilizar el formato json para recibir y enviar datos
en la API, aunque una opción era enviar bytes directamente fue más
sencillo implementar json en ambas operaciones con el uso de
libcurl y la herramienta jsonify de Flask.

-   Es recomendado no castear la data enviada como string a bytes, esto
debido a que el uso de la biblioteca base64 en python puede
recibir un string y devolverá los bytes, esto aplica tanto para la
codificación como para la codificación.

-   Es recomendable utilizar connect y luego sendall en lugar de sendto
en lo que se refiere a la biblioteca de python socket, sendto
presentaba algunas incongruencias al intentar conectarse a
distintos DNS remotos, connect es más estable.

-   Se recomienda construir y levantar una sola vez elasticsearch y
kibana (iniciarlos desde Portainer), esto porque al estar
cambiando los archivos .c del DNS interceptor, se vuelve tedioso
estar iniciando estos dos containers una y otra vez cuando no es
necesario, pues solamente son APIs.

-   Se recomienda utilizar la herramienta de nslookup para comprobar el
funcionamiento del DNS interceptor.

-   Se recomienda utilizar operaciones bitwise para obtener porciones de
archivos o datos binarios, pues consumen menos ciclos de cpu.

-   Se recomienda el uso de elasticsearch (base de datos no relacional),
para almacenar hostnames e IPs de manera más sencilla.

-   Se recomienda transformar los request de DNS a base 64 al pasarlos
al DNS API y de vuelta, pues al estar en binario y no ser
caracteres ASCII visibles, podría generar pérdida de paquetes a la
hora de pasarlo del DNS interceptor al DNS API y viceversa.

## Referencias
Ali, A., 2022. *Base64 Encoding and Decoding Using Python*. [online]
Code Envato Tuts+. Disponible en:
<https://code.tutsplus.com/tutorials/base64-encoding-and-decoding-using-python--cms-25588>
[Accedido el 27 de Mayo de 2022].

Datatracker.ietf.org. 1987. *DOMAIN NAMES - IMPLEMENTATION AND
SPECIFICATION | Datatracker.ietf.org*. [online] Disponible en:
<https://datatracker.ietf.org/doc/html/rfc1035#section-4.1>
[Accedido el 31 de Mayo de 2022].

Docs.python.org. 2022. *socket --- Low-level networking interface ---
Python 3.10.4 documentation*. [online] Disponible en:
<https://docs.python.org/3/library/socket.html> [Accedido el 27 de
Mayo de 2022].

Nachtimwald.com 2017. *Base64 Encode and Decode in C Pythontic.com*.
[online] Disponible en:
<https://nachtimwald.com/2017/11/18/base64-encode-and-decode-in-c/>
[Accedido el 27 de Mayo de 2022].

Nakita, P., 2022. *Docker Tutorial - basic setup a Python Flask
Application in a Docker container*. [online] Youtube.com. Disponible
en: <https://www.youtube.com/watch?v=GVs26OxzE3o> [Accedido el 27 de
Mayo de 2022].

Pythontic.com. 2022. *UDP - Client and Server example programs in Python
| Pythontic.com*. [online] Disponible en:
<https://pythontic.com/modules/socket/udp-client-server-example>
[Accedido el 27 de Mayo de 2022].
