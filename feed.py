import asyncio
import websockets
import json
import socket
import time

# UDP config (C++ engine)
UDP_IP = "127.0.0.1"
UDP_PORT = 5555
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

# Oxirgi orderbook (duplicate filter uchun)
last_depth = None

async def binance_ws():
    global last_depth
    
    url = "wss://stream.binance.com:9443/ws/btcusdt@depth10@100ms/btcusdt@aggTrade"
    
    while True:
        try:
            print("[*] Binance WebSocket'ga ulanmoqda...")
            
            async with websockets.connect(url, ping_interval=20, ping_timeout=10) as ws:
                print("[+] Ulandi! Ma'lumot C++ dvigateliga (UDP:5555) uzatilmoqda 🚀")
                
                while True:
                    msg = await ws.recv()
                    data = json.loads(msg)

                    # =========================
                    # 🔹 TRADE (aggTrade)
                    # =========================
                    if data.get('e') == 'aggTrade':
                        ts = data.get('T', int(time.time() * 1000))  # exchange time
                        p = data['p']
                        q = data['q']
                        m = 1 if data['m'] else 0
                        
                        out = f"T,{ts},{p},{q},{m}"
                        sock.sendto(out.encode('ascii'), (UDP_IP, UDP_PORT))

                    # =========================
                    # 🔹 ORDERBOOK (Depth)
                    # =========================
                    elif 'bids' in data and 'asks' in data:
                        ts = data.get('E', int(time.time() * 1000))

                        bids = data['bids'][:10]
                        asks = data['asks'][:10]

                        # 🔥 Duplicate filter
                        current = (
                            tuple(map(tuple, bids)),
                            tuple(map(tuple, asks))
                        )

                        if current == last_depth:
                            continue  # bir xil bo‘lsa tashlab yuboramiz

                        last_depth = current

                        # 🔧 Formatlash
                        parts = [f"D,{ts}"]

                        # BIDS
                        for b in bids:
                            parts.append(f"{b[0]},{b[1]}")
                        for _ in range(10 - len(bids)):
                            parts.append("0,0")

                        # ASKS
                        for a in asks:
                            parts.append(f"{a[0]},{a[1]}")
                        for _ in range(10 - len(asks)):
                            parts.append("0,0")

                        out = ",".join(parts)
                        sock.sendto(out.encode('ascii'), (UDP_IP, UDP_PORT))

        except Exception as e:
            print(f"[!] Uzilish: {e}")
            print("[*] 3 soniyadan so‘ng qayta ulanadi...")
            await asyncio.sleep(3)

if __name__ == "__main__":
    asyncio.run(binance_ws())