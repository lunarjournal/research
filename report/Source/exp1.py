import asyncio
import websockets
import threading
import ssl
import pathlib
import readline

sockets = []
index = 0
selector = 0

async def main():
	while True:
		cmd = input()
		split = cmd.split();
		if(split[0] == 'set'):
			selector = int(split[1])
			print("channel " + str(selector))
		else:
			try:
				await sockets[selector].send("".join(cmd))
			except:
				pass
		
def bind():
    loop = asyncio.new_event_loop()
    asyncio.set_event_loop(loop)
    loop.run_until_complete(main())
    loop.close()

async def hello(websocket, path):
	sockets.append(websocket)
	global index
	id = index
	print(str(id) + " connected")
	print(str(id) + " --> " + str(websocket.remote_address[0]))
	index+=1
	try:
		await websocket.send("window.location.href")
	except:
		pass
	while(1):
		try:
			name = await websocket.recv()
			print(str(id) + " --> " + name)
		except:
			print(str(id) + " disconnected")
			break

ssl_context = ssl.SSLContext(ssl.PROTOCOL_TLS_SERVER)
cert_pem = pathlib.Path(__file__).with_name("./fullchain.pem")
key_pem = pathlib.Path(__file__).with_name("./privkey.pem")
ssl_context.load_cert_chain(certfile=cert_pem, keyfile=key_pem)


start_server = websockets.serve(hello, "0.0.0.0", 4444, ssl=ssl_context)

asyncio.get_event_loop().run_until_complete(start_server)
t = threading.Thread(target=bind)
t.start()

asyncio.get_event_loop().run_forever()