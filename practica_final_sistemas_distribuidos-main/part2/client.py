from enum import Enum
import socket
import argparse
import time
import threading
import os
import traceback
import sys
import requests

def call_date_time_service():
    try:
        response = requests.get('http://127.0.0.1:5000/date-time')  # Assuming Flask is running locally on port 5000

        status_code = response.status_code
        response_data = response.text

        return status_code, response_data.encode('utf-8')
    except Exception as e:
        return None, str(e)


server_active = False

# Socket object for accepting connections
global_socket = None

def accept_connections():
    global server_active, global_socket
    try:
        while server_active:
            try:
                # Accept an incoming connection
                client_socket, client_address = global_socket.accept()
                # Recibir el mensaje del cliente
                file_path = client_socket.recv(32 * 1024)
                file_path = file_path.decode("utf-8")
                file_path = file_path[:-1]
                try:
                    # Abrir el archivo y leer su contenido en bytes
                    with open(file_path, 'rb') as file:
                        file_contents = file.read()
                    # Enviar el contenido del archivo de vuelta a través del socket
                    client_socket.sendall(b'0')
                    time.sleep(0.1)
                    client_socket.sendall(file_contents)
                    #print("server File sent successfully")
                except FileNotFoundError:
                    client_socket.sendall(b'1')
                    #print(" server c > GET_FILE FAIL / FILE NOT EXIST")
                finally:
                    client_socket.close()
                    #print(" server Client connection closed")
            except Exception as e:
                client_socket.sendall(b'2')
                print("Error:", e)
    except Exception as e:
        client_socket.sendall(b'2')
        print("Error:", e)

def start_server():
    global global_socket, server_active
    server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    try:
        HOST = socket.gethostbyname(socket.gethostname())
        PORT = 0
        server_socket.bind((HOST, PORT))
        server_socket.listen()
        assigned_port = server_socket.getsockname()[1]
        global_socket = server_socket
        server_active = True
        accept_thread = threading.Thread(target=accept_connections)
        accept_thread.daemon = True
        accept_thread.start()
        return HOST, assigned_port
    except Exception as e:
        print("Error:", e)
        if server_socket:
            server_socket.close()

def stop_server():
    global global_socket, server_active
    if global_socket:
        global_socket.close()
    server_active = False

class client :

    # ******************** TYPES *********************
    # *
    # * @brief Return codes for the protocol methods
    class RC(Enum) :
        OK = 0
        ERROR = 1
        USER_ERROR = 2
        OTHER_ERROR = 3
        ANOTHER_ERROR = 4


    # ****************** ATTRIBUTES ******************
    # Initialize attributes
    _address = None
    _server = None
    _port = -1
    _user = None


    # ******************** METHODS *******************


    @staticmethod
    def  register(user) :
        try:
            # Conectar al servidor
            client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            client_socket.connect((client._address, client._port))
            # Enviar la operación REGISTER al servidor
            status_code, response_data = call_date_time_service()
            if status_code != 200:
                print("c> REGISTER FAIL")
                return client.RC.USER_ERROR

            print("la response data es: ", response_data )
            # Envía el comando 'REGISTER' al servidor
            client_socket.sendall(b'REGISTER' + b'\0')
            time.sleep(0.1)

            client_socket.sendall(response_data + b'\0')
            time.sleep(0.1)

            # Envía el nombre de usuario al servidor
            client_socket.sendall(user.encode('utf-8') + b'\0')

            time.sleep(0.1)

            # Recibir el resultado del servidor
            result = client_socket.recv(1)

            # Cerrar la conexión
            client_socket.close()

            # Interpretar el resultado recibido
            if result == b'\x00':
                client._user = user
                print("c> REGISTER OK")
                return client.RC.OK
            elif result == b'\x01':
                print("c> USERNAME IN USE")
                return client.RC.ERROR
            elif result == b'\x02':
                print("c> REGISTER FAIL")
                return client.RC.USER_ERROR

        except Exception as e:
            print("Error:", e)
            return client.RC.USER_ERROR

   
    @staticmethod
    def  unregister(user) :
        try:
            # Conectar al servidor
            client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            client_socket.connect((client._address, client._port))

            # Enviar la operación UNREGISTER al servidor
            client_socket.sendall(b'UNREGISTER'+ b'\0')

            time.sleep(0.1)

            # Enviar el nombre de usuario al servidor
            client_socket.sendall(user.encode('utf-8')+ b'\0')

            time.sleep(0.1)

            # Recibir el resultado del servidor
            result = client_socket.recv(1)

            # Cerrar la conexión
            client_socket.close()

            # Interpretar el resultado recibido
            if result == b'\x00':
                print("c> UNREGISTER OK")
                client._user = None
                return client.RC.OK
            elif result == b'\x01':
                print("c> USER DOES NOT EXIST")
                return client.RC.ERROR
            elif result == b'\x02':
                print("c> UNREGISTER FAIL")
                return client.RC.USER_ERROR

        except Exception as e:
            print("Error:", e)
            return client.RC.USER_ERROR

    
    @staticmethod
    def  connect( user) :
        try:
            # Conectar al servidor
            address, port = start_server()
            client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            client_socket.connect((client._address, client._port))

            # Enviar la operación CONNECT al servidor
            client_socket.sendall(b'CONNECT'+ b'\0')

            time.sleep(0.1)

            # Enviar el nombre de usuario al servidor
            client_socket.sendall(user.encode('utf-8')+ b'\0')

            time.sleep(0.1)

            # Enviar el ip
            client_socket.sendall(address.encode('utf-8')+ b'\0')

            time.sleep(0.1)

            # Enviar el el puerto
            client_socket.sendall(str(port).encode('utf-8') + b'\0')

            time.sleep(0.1)

            # Recibir el resultado del servidor
            result = client_socket.recv(1)

            # Cerrar la conexión
            client_socket.close()

            # Interpretar el resultado recibido
            if result == b'\x00':
                print("c> CONNECT OK")
                client._user = user
                return client.RC.OK
            elif result == b'\x01':
                print("c> CONNECT FAIL, USER DOES NOT EXIST")
                stop_server()
                return client.RC.ERROR
            elif result == b'\x02':
                print("c> USER ALREADY CONNECTED")
                client._user = user
                stop_server()
                return client.RC.USER_ERROR
            elif result == b'\x03':
                print("c> CONNECT FAIL")
                stop_server()
                return client.RC.OTHER_ERROR

            
        except Exception as e:
            print("Error:", e)
            return client.RC.OTHER_ERROR
        return client.RC.OTHER_ERROR
            
    
    @staticmethod
    def  disconnect(user) :
        try:
            # Conectar al servidor
            client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            client_socket.connect((client._address, client._port))

            # Enviar la operación DISCONNECT al servidor
            client_socket.sendall(b'DISCONNECT'+ b'\0')

            time.sleep(0.1)

            # Enviar el nombre de usuario al servidor
            client_socket.sendall(user.encode('utf-8')+ b'\0')

            time.sleep(0.1)

            # Recibir el resultado del servidor
            result = client_socket.recv(1)

            # Cerrar la conexión
            client_socket.close()

            # Interpretar el resultado recibido
            if result == b'\x00':
                print("c> DISCONNECT OK")
                client._user == None
                stop_server()
                return client.RC.OK
            elif result == b'\x01':
                print("c> DISCONNECT FAIL / USER DOES NOT EXIST")
                return client.RC.ERROR
            elif result == b'\x02':
                print("c> DISCONNECT FAIL / USER NOT CONNECTED")
                return client.RC.USER_ERROR
            elif result == b'\x03':
                print("c> DISCONNECT FAIL")
                return client.RC.OTHER_ERROR

        except Exception as e:
            print("Error:", e)
            return client.RC.OTHER_ERROR
        return client.RC.OTHER_ERROR


    @staticmethod
    def publish(fileName, description):
        try:
            # Conectar al servidor
            if client._user is not None:
                client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
                client_socket.connect((client._address, client._port))

                # Enviar la operación PUBLISH al servidor
                client_socket.sendall(b'PUBLISH'+ b'\0')

                time.sleep(0.1)

                # Enviar el nombre del usuario al servidor
                client_socket.sendall(client._user.encode('utf-8')+ b'\0')

                time.sleep(0.1)

                # Enviar el nombre del archivo al servidor
                client_socket.sendall(fileName.encode('utf-8')+ b'\0')

                time.sleep(0.1)

                # Enviar la descripción al servidor
                client_socket.sendall(description.encode('utf-8')+ b'\0')

                time.sleep(0.1)

                # Recibir el resultado del servidor
                result = client_socket.recv(1)

                # Cerrar la conexión
                client_socket.close()

                # Interpretar el resultado recibido
                if result == b'\x00':
                    print("c> PUBLISH OK")
                    return client.RC.OK
                elif result == b'\x01':
                    print("c> PUBLISH FAIL, USER DOES NOT EXIST")
                    return client.RC.ERROR
                elif result == b'\x02':
                    print("c> PUBLISH FAIL, USER NOT CONNECTED")
                    return client.RC.USER_ERROR
                elif result == b'\x03':
                    print("c> PUBLISH FAIL, CONTENT ALREADY PUBLISHED")
                    return client.RC.OTHER_ERROR
                elif result == b'\x04':
                    print("c> PUBLISH FAIL")
                    return client.RC.ANOTHER_ERROR
            else:
                print("c> PUBLISH FAIL, USER DOES NOT EXIST")
                return client.RC.USER_ERROR

        except Exception as e:
            print("Error:", e)
            return client.RC.ANOTHER_ERROR

    @staticmethod
    def  delete(fileName) :
        try:
            if client._user is not None:
                # Conectar al servidor
                client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
                client_socket.connect((client._address, client._port))

                # Enviar la operación DELETE al servidor
                client_socket.sendall(b'DELETE'+ b'\0')

                time.sleep(0.1)

                # Enviar el nombre del usuario al servidor
                client_socket.sendall(client._user.encode('utf-8')+ b'\0')

                time.sleep(0.1)

                # Enviar el nombre del archivo al servidor
                client_socket.sendall(fileName.encode('utf-8')+ b'\0')

                time.sleep(0.1)

                # Recibir el resultado del servidor
                result = client_socket.recv(1)

                # Cerrar la conexión
                client_socket.close()

                # Interpretar el resultado recibido
                if result == b'\x00':
                    print("c> DELETE OK")
                    return client.RC.OK
                elif result == b'\x01':
                    print("c> DELETE FAIL, USER DOES NOT EXIST")
                    return client.RC.ERROR
                elif result == b'\x02':
                    print("c> DELETE FAIL, USER NOT CONNECTED")
                    return client.RC.USER_ERROR
                elif result == b'\x03':
                    print("c> DELETE FAIL, CONTENT NOT PUBLISHED")
                    return client.RC.OTHER_ERROR
                elif result == b'\x04':
                    print("c> DELETE FAIL")
                    return client.RC.ANOTHER_ERROR
            else:
                print("c> DELETE FAIL, USER DOES NOT EXIST")
                return client.RC.USER_ERROR

        except Exception as e:
            print("Error:", e)
            return client.RC.ANOTHER_ERROR

    @staticmethod
    def  listusers(option=None) :
        try:
            if client._user is not None:
                # Conectar al servidor
                client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
                client_socket.connect((client._address, client._port))

                # Enviar la operación LIST_USERS al servidor
                client_socket.sendall(b'LIST_USERS'+ b'\0')

                time.sleep(0.1)

                # Enviar el nombre del usuario al servidor
                client_socket.sendall(client._user.encode('utf-8')+ b'\0')

                time.sleep(0.1)

                # Recibir el resultado del servidor
                result = client_socket.recv(1)


                # Interpretar el resultado recibido
                if result == b'\x00':
                    if option is None:
                        print("c> LIST_USERS OK")
                    data = client_socket.recv(1024)
                    # Decodificar la cadena de bytes a una cadena de texto
                    decoded_data = data.decode('utf-8')

                    # Separar las cadenas de texto de cada usuario
                    users_data = decoded_data.split('\x00')

                    # Eliminar la cadena vacía al principio y al final (debido a los bytes nulos)
                    users_data = users_data[3:-1]

                    # Parsear los datos de cada usuario y almacenarlos en una lista de diccionarios
                    users_info = []
                    for user_data in users_data:
                        user_info = user_data.split(' ')
                        username = user_info[0]
                        ip_address = user_info[1]
                        port = user_info[2]
                        user_dict = {'username': username, 'ip_address': ip_address, 'port': port}
                        users_info.append(user_dict)

                    # Imprimir la información de cada usuario
                    for user in users_info:
                         if option is None:
                            print("\t{} {} {}".format(user['username'], user['ip_address'], user['port']))
                    return users_data
                elif result == b'\x01':
                    if option is None:
                        print("c>  LIST_USERS FAIL, USER DOES NOT EXIST")
                    return client.RC.ERROR
                elif result == b'\x02':
                    if option is None:
                        print("c> LIST_USERS FAIL, USER NOT CONNECTED")
                    return client.RC.USER_ERROR
                elif result == b'\x03':
                    if option is None:
                        print("c> LIST_USERS FAIL")
                    return client.RC.OTHER_ERROR
            # Cerrar la conexión
            else:
                if option is None:
                    print("c>  LIST_USERS FAIL, USER DOES NOT EXIST")
                
                return client.RC.USER_ERROR
            
            client_socket.close()
            
            
        except Exception as e:
            print("Error:", e)
            print("sie esta aqui es porque efectivamente hay un error, si el usuario icluye esacios da error")
            return client.RC.OTHER_ERROR
        return client.RC.OTHER_ERROR

    @staticmethod
    def  listcontent(user) :
        try:
            if client._user is not None:
                # Conectar al servidor
                client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
                client_socket.connect((client._address, client._port))

                # Enviar la operación LIST_USERS al servidor
                client_socket.sendall(b'LIST_CONTENT'+ b'\0')

                time.sleep(0.1)

                # Enviar el nombre del usuario al servidor
                client_socket.sendall(client._user.encode('utf-8')+ b'\0')

                time.sleep(0.1)

                # Enviar el nombre del usuario del que se desea saber su contenido
                client_socket.sendall(user.encode('utf-8')+ b'\0')

                time.sleep(0.1)

                # Recibir el resultado del servidor
                result = client_socket.recv(1)


                # Interpretar el resultado recibido
                if result == b'\x00':
                    print("LIST_CONTENT OK")
                    data = client_socket.recv(1024)
                    # Decodificar la cadena de bytes a una cadena de texto
                    decoded_data = data.decode('utf-8')

                    # Separar las cadenas de texto de cada usuario
                    content_data = decoded_data.split('\x00')

                    # Eliminar la cadena vacía al principio y al final (debido a los bytes nulos)
                    content_data = content_data[3:-1]
                    
                    content_info = []
                    # Iterar sobre cada elemento de content_data
                    for content in content_data:
                    # Separar la cadena de contenido en nombre de archivo y descripción
                        file_info = content.split(' ')
                        filename = file_info[0]
                        description = file_info[1]
                        content_dict = {'filename': filename, 'description': description}
                        content_info.append(content_dict)

                    for file in content_info:
                        print("\t{} {}".format(file['filename'], file['description']))
                    return client.RC.OK
                    
                elif result == b'\x01':
                    print("c> LIST_CONTENT FAIL, USER DOES NOT EXIST")
                    return client.RC.ERROR
                elif result == b'\x02':
                    print("c>  LIST_CONTENT FAIL, USER NOT CONNECTED")
                    return client.RC.USER_ERROR
                elif result == b'\x03':
                    print("c>  LIST_CONTENT FAIL, REMOTE USER DOES NOT EXIST")
                    return client.RC.OTHER_ERROR
                elif result == b'\x04':
                    print("c> LIST_CONTENT FAIL")
                    return client.RC.ANOTHER_ERROR
            else:
                print("c> LIST_CONTENT FAIL, USER DOES NOT EXIST")
                return client.RC.USER_ERROR
            # Cerrar la conexión
            client_socket.close()

        except Exception as e:
            print("Error:", e)
            return client.RC.ANOTHER_ERROR

    
    def  getfile( user,  remote_FileName,  local_FileName) :

        try:
            if client._user is not None:
                connected_users = client.listusers(1)


                for user_data in connected_users:
                    user_info = user_data.split(' ')
                    if user == user_info[0]:
                        user_ip_address = user_info[1]
                        user_port = user_info[2]
                        client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
                        client_socket.connect((user_ip_address, int(user_port)))

                        # Enviar el nombre del archivo al servidor
                        client_socket.sendall(remote_FileName.encode('utf-8')+ b'\0')

                        result = client_socket.recv(1)
                        if result == b'0':

                            # Recibir el contenido del archivo
                            with open(local_FileName, 'wb') as f:
                                while True:
                                    data = client_socket.recv(32 * 1024)
                                    if not data:
                                        break
                                    f.write(data)
                            print("c> GET_FILE OK")
                            return client.RC.OK  # Indicar que la operación se realizó correctamente
                        elif result == b'1':
                            print("c>  GET_FILE FAIL / FILE NOT EXIST")
                            return client.RC.ERROR
                        elif result == b'2':
                            print("c> GET_FILE FAIL")
                            return client.RC.USER_ERROR
                else:
                    print("c> GET_FILE FAIL")
                    return client.RC.USER_ERROR

        except Exception as e:
            print("c> GET_FILE FAIL")
            return client.RC.ANOTHER_ERROR        

    # *
    # **
    # * @brief Command interpreter for the client. It calls the protocol functions.
    @staticmethod
    def shell():

        while (True) :
            try :
                command = input("c> ")
                line = command.split(" ")
                if (len(line) > 0):

                    line[0] = line[0].upper()

                    if (line[0]=="REGISTER") :
                        if (len(line) == 2) :
                            client.register(line[1])
                        else :
                            print("Syntax error. Usage: REGISTER <userName>")

                    elif(line[0]=="UNREGISTER") :
                        if (len(line) == 2) :
                            client.unregister(line[1])
                        else :
                            print("Syntax error. Usage: UNREGISTER <userName>")

                    elif(line[0]=="CONNECT") :
                        if (len(line) == 2) :
                            client.connect(line[1])
                        else :
                            print("Syntax error. Usage: CONNECT <userName>")
                    
                    elif(line[0]=="PUBLISH") :
                        if (len(line) >= 3) :
                            #  Remove first two words
                            description = ' '.join(line[2:])
                            client.publish(line[1], description)
                        else :
                            print("Syntax error. Usage: PUBLISH <fileName> <description>")

                    elif(line[0]=="DELETE") :
                        if (len(line) == 2) :
                            client.delete(line[1])
                        else :
                            print("Syntax error. Usage: DELETE <fileName>")

                    elif(line[0]=="LIST_USERS") :
                        if (len(line) == 1) :
                            client.listusers()
                        else :
                            print("Syntax error. Use: LIST_USERS")

                    elif(line[0]=="LIST_CONTENT") :
                        if (len(line) == 2) :
                            client.listcontent(line[1])
                        else :
                            print("Syntax error. Usage: LIST_CONTENT <userName>")

                    elif(line[0]=="DISCONNECT") :
                        if (len(line) == 2) :
                            client.disconnect(line[1])
                        else :
                            print("Syntax error. Usage: DISCONNECT <userName>")

                    elif(line[0]=="GET_FILE") :
                        if (len(line) == 4) :
                            client.getfile(line[1], line[2], line[3])
                        else :
                            print("Syntax error. Usage: GET_FILE <userName> <remote_fileName> <local_fileName>")

                    elif(line[0]=="QUIT") :
                        if (len(line) == 1) :
                            if server_active:
                                stop_server()
                                print("c> May the force be with you!")
                                sys.exit()
                            else:
                                print("c> May the force be with you!")
                                sys.exit()
                        else :
                            print("Syntax error. Use: QUIT")
                    else :
                        print("Error: command " + line[0] + " not valid.")

            except Exception as e:
                print("Exception: " + str(e))
                exc_type, exc_obj, exc_tb = sys.exc_info()
                fname = os.path.split(exc_tb.tb_frame.f_code.co_filename)[1]
                print("Exception: " + str(e))
                print("File:", fname)
                print("Line:", exc_tb.tb_lineno)
                traceback.print_exc()

    # *
    # * @brief Prints program usage
    @staticmethod
    def usage() :
        print("Usage: python3 client.py -s <server> -p <port>")


    # *
    # * @brief Parses program execution arguments
    @staticmethod
    def  parseArguments(argv) :
        parser = argparse.ArgumentParser()
        parser.add_argument('-s', type=str, required=True, help='Server IP')
        parser.add_argument('-p', type=int, required=True, help='Server Port')
        args = parser.parse_args()

        if (args.s is None):
            parser.error("Usage: python3 client.py -s <server> -p <port>")
            return False

        if ((args.p < 1024) or (args.p > 65535)):
            parser.error("Error: Port must be in the range 1024 <= port <= 65535");
            return False;
        
        client._address = args.s
        client._port = args.p

        return True


    # ******************** MAIN *********************
    @staticmethod
    def main(argv) :
        if (not client.parseArguments(argv)) :
            client.usage()
            return

        #  Write code here
        client.shell()
        print("+++ FINISHED +++")
    

if __name__=="__main__":
    client.main([])