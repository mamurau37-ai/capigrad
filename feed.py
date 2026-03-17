import asyncio
import websockets
import json
import socket
import time

# C++ ga ma'lumot otish uchun port
UDP_IP = "127.0.0.1"
UDP_PORT = 5555
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

async def binance_ws():
    url = "wss://stream.binance.com:9443/ws/btcusdt@depth10@100ms/btcusdt@aggTrade"
    while True:
        try:
            print("[*] Binance WebSocket'ga ulanmoqda...")
            async with websockets.connect(url) as ws:
                print("[+] Ulandi! Ma'lumot C++ dvigateliga (UDP:5555) uzatilmoqda 🚀")
                while True:
                    msg = await ws.recv()
                    data = json.loads(msg)
                    ts = int(time.time() * 1000)
                    
                    # Agar Trade kelsa
                    if 'e' in data and data['e'] == 'aggTrade':
                        p = data['p']
                        q = data['q']
                        m = 1 if data['m'] else 0
                        out = f"T,{ts},{p},{q},{m}"
                        sock.sendto(out.encode(), (UDP_IP, UDP_PORT))
                        
                    # Agar Stakan (Depth) kelsa
                    elif 'bids' in data and 'asks' in data:
                        bids = data['bids'][:10]
                        asks = data['asks'][:10]
                        
                        out = f"D,{ts}"
                        for b in bids: out += f",{b[0]},{b[1]}"
                        for _ in range(10 - len(bids)): out += ",0,0" # Yetmasa to'ldirish
                            
                        for a in asks: out += f",{a[0]},{a[1]}"
                        for _ in range(10 - len(asks)): out += ",0,0"
                            
                        sock.sendto(out.encode(), (UDP_IP, UDP_PORT))
                        
        except Exception as e:
            print(f"[!] Uzilish ro'y berdi: {e}. 3 soniyadan so'ng qayta ulanadi...")
            await asyncio.sleep(3)

if __name__ == "__main__":
    asyncio.run(binance_ws())