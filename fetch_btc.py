#!/usr/bin/env python3
"""
CGT Data Fetcher v1.0
Zanerhon's Binance OHLCV yuklovchi + filtrlash

Nima qiladi:
  1. Binance Futures API dan 1 yillik BTC/USDT 1h data yuklab oladi
  2. Ma'lumotni tozalaydi (NaN, nol, anomaliyalar)
  3. Filtrlaydi (past volume, katta gap, duplicate)
  4. btc_1h_1year.csv ga saqlaydi

Ishlatish:
  python3 fetch_btc.py                      # BTC 1h 5 yil
  python3 fetch_btc.py --interval 15m       # BTC 15m 5 yil
  python3 fetch_btc.py --days 365           # BTC 1h 1 yil
  python3 fetch_btc.py --symbol ETHUSDT     # ETH 1h 5 yil
  python3 fetch_btc.py --interval 1h --days 1825   # aniq 5 yil
"""

import urllib.request
import urllib.parse
import json
import csv
import time
import sys
import os
from datetime import datetime, timezone

# ── Argumentlar ───────────────────────────────────────────────────
SYMBOL   = "BTCUSDT"
INTERVAL = "1h"
DAYS     = 1825  # 5 yil (Binance futures 2019-09 dan)
OUTPUT   = None  # None = avtomatik nom

for i, arg in enumerate(sys.argv[1:], 1):
    if arg == "--symbol"   and i < len(sys.argv): SYMBOL   = sys.argv[i+1] if i < len(sys.argv)-1 else SYMBOL
    if arg == "--interval" and i < len(sys.argv): INTERVAL = sys.argv[i+1] if i < len(sys.argv)-1 else INTERVAL
    if arg == "--days"     and i < len(sys.argv): DAYS     = int(sys.argv[i+1]) if i < len(sys.argv)-1 else DAYS
    if arg == "--output"   and i < len(sys.argv): OUTPUT   = sys.argv[i+1] if i < len(sys.argv)-1 else OUTPUT

if OUTPUT is None:
    years = DAYS // 365
    OUTPUT = f"{SYMBOL.lower()}_{INTERVAL}_{years}year.csv"
    if years <= 1:
        OUTPUT = f"{SYMBOL.lower()}_{INTERVAL}_{DAYS}d.csv"

# Binance Futures REST (API key shart emas public data uchun)
BASE_URL = "https://fapi.binance.com"

# ── Interval ms ───────────────────────────────────────────────────
INTERVAL_MS = {
    "1m":  60_000,
    "3m":  180_000,
    "5m":  300_000,
    "15m": 900_000,
    "30m": 1_800_000,
    "1h":  3_600_000,
    "2h":  7_200_000,
    "4h":  14_400_000,
    "1d":  86_400_000,
}
iv_ms = INTERVAL_MS.get(INTERVAL, 3_600_000)

# ════════════════════════════════════════════════════════════════
#  BINANCE API
# ════════════════════════════════════════════════════════════════
def fetch_klines(symbol, interval, start_ms, limit=1000):
    """Binance dan OHLCV yuklash"""
    params = urllib.parse.urlencode({
        "symbol":    symbol,
        "interval":  interval,
        "startTime": start_ms,
        "limit":     limit,
    })
    url = f"{BASE_URL}/fapi/v1/klines?{params}"
    try:
        req = urllib.request.Request(url, headers={"User-Agent": "CGT/3.0"})
        with urllib.request.urlopen(req, timeout=15) as resp:
            data = json.loads(resp.read().decode())
            return data
    except Exception as e:
        print(f"\n[!] Xato: {e}")
        return []

def check_connection():
    """Binance ulanishni tekshirish"""
    try:
        url = f"{BASE_URL}/fapi/v1/ping"
        req = urllib.request.Request(url, headers={"User-Agent": "CGT/3.0"})
        with urllib.request.urlopen(req, timeout=10) as resp:
            resp.read()
        return True
    except:
        return False

def get_server_time():
    """Binance server vaqti"""
    try:
        url = f"{BASE_URL}/fapi/v1/time"
        req = urllib.request.Request(url, headers={"User-Agent": "CGT/3.0"})
        with urllib.request.urlopen(req, timeout=10) as resp:
            data = json.loads(resp.read().decode())
            return data["serverTime"]
    except:
        return int(time.time() * 1000)

# ════════════════════════════════════════════════════════════════
#  FILTRLASH VA TOZALASH
# ════════════════════════════════════════════════════════════════
def clean_candles(raw_data):
    """
    Raw Binance klines ni tozalashpython3 fetch_btc.py --interval 15m

    Format: [ts, open, high, low, close, volume, ...]
    """
    candles = []
    for row in raw_data:
        try:
            ts     = int(row[0])
            open_  = float(row[1])
            high   = float(row[2])
            low    = float(row[3])
            close  = float(row[4])
            volume = float(row[5])

            # Asosiy tekshiruvlar
            if close <= 0 or open_ <= 0: continue
            if high < low:               continue
            if high < close:             continue
            if low  > close:             continue
            if volume < 0:               continue

            candles.append({
                "ts":     ts,
                "open":   open_,
                "high":   high,
                "low":    low,
                "close":  close,
                "volume": volume,
            })
        except:
            continue
    return candles

def filter_candles(candles, verbose=True):
    """
    Filtrlash:
      1. Duplicate timestamp olib tashlash
      2. Nol yoki juda past volume (bot candle)
      3. Katta narx gaplari (anomaliya)
      4. Juda kichik sham (spread < 0.001%)
    """
    if not candles:
        return candles

    original_count = len(candles)
    removed = {"duplicate": 0, "low_volume": 0, "price_gap": 0, "zero_range": 0}

    # 1. Duplicate timestamp olib tashlash
    seen_ts = set()
    unique = []
    for c in candles:
        if c["ts"] not in seen_ts:
            seen_ts.add(c["ts"])
            unique.append(c)
        else:
            removed["duplicate"] += 1
    candles = unique

    # 2. Volume statistikasi hisoblash
    volumes = [c["volume"] for c in candles]
    volumes.sort()
    p5_vol  = volumes[len(volumes)//20]   # 5-percentil
    avg_vol = sum(volumes) / len(volumes)

    # 3. Closes
    closes = [c["close"] for c in candles]

    filtered = []
    for i, c in enumerate(candles):

        # Past volume filtr (5 percentildan past = bot/dead candle)
        if c["volume"] < p5_vol * 0.1:
            removed["low_volume"] += 1
            continue

        # Narx gap filtr: oldingi close dan 10% dan ko'p farq
        if i > 0:
            prev_close = candles[i-1]["close"]
            gap_pct = abs(c["open"] - prev_close) / prev_close
            if gap_pct > 0.10:  # 10% dan katta gap = anomaliya
                removed["price_gap"] += 1
                continue

        # Juda kichik sham range (wash trading belgisi)
        range_pct = (c["high"] - c["low"]) / c["close"]
        if range_pct < 0.00005:  # 0.005% dan kichik
            removed["zero_range"] += 1
            continue

        filtered.append(c)

    if verbose:
        print(f"\n  📊 Filtrlash natijasi:")
        print(f"     Asl:         {original_count:,}")
        print(f"     Duplicate:  -{removed['duplicate']}")
        print(f"     Past volume:{-removed['low_volume']}")
        print(f"     Gap anomaly:{-removed['price_gap']}")
        print(f"     Zero range: -{removed['zero_range']}")
        print(f"     Natija:      {len(filtered):,} sham ✓")

    return filtered

def add_quality_score(candles):
    """
    Har shamga sifat balli qo'shish (0-100)
    Model training uchun og'irlik sifatida ishlatish mumkin
    """
    if not candles:
        return candles

    volumes = [c["volume"] for c in candles]
    avg_vol = sum(volumes) / len(volumes)

    for i, c in enumerate(candles):
        score = 100

        # Volume sifati
        vol_ratio = c["volume"] / avg_vol
        if vol_ratio < 0.3:   score -= 30
        elif vol_ratio < 0.5: score -= 10
        elif vol_ratio > 5.0: score -= 15  # juda yuqori = anomaliya

        # OHLC mantiqiyligi
        if c["high"] - c["low"] < c["close"] * 0.001: score -= 20

        # Gap penalty
        if i > 0:
            gap = abs(c["open"] - candles[i-1]["close"]) / candles[i-1]["close"]
            if gap > 0.03: score -= int(gap * 100)

        c["quality"] = max(0, min(100, score))

    return candles

def print_statistics(candles):
    """Data statistikasi"""
    if not candles:
        return

    closes  = [c["close"] for c in candles]
    volumes = [c["volume"] for c in candles]
    returns = [(closes[i]-closes[i-1])/closes[i-1]*100
               for i in range(1, len(closes))]

    avg_vol = sum(volumes) / len(volumes)
    avg_ret = sum(returns) / len(returns) if returns else 0
    std_ret = (sum((r-avg_ret)**2 for r in returns)/len(returns))**0.5 if returns else 0

    pos_ret = sum(1 for r in returns if r > 0)
    neg_ret = sum(1 for r in returns if r < 0)

    ts_start = datetime.fromtimestamp(candles[0]["ts"]/1000, tz=timezone.utc)
    ts_end   = datetime.fromtimestamp(candles[-1]["ts"]/1000, tz=timezone.utc)

    print(f"\n  📈 Data statistikasi:")
    print(f"     Davr:        {ts_start.strftime('%Y-%m-%d')} → {ts_end.strftime('%Y-%m-%d')}")
    print(f"     Shamlar:     {len(candles):,}")
    print(f"     Min narx:    ${min(closes):,.1f}")
    print(f"     Max narx:    ${max(closes):,.1f}")
    print(f"     Joriy:       ${closes[-1]:,.1f}")
    print(f"     O'rt volume: {avg_vol:,.0f} USDT")
    print(f"     O'rt return: {avg_ret:+.4f}%")
    print(f"     Volatility:  {std_ret:.4f}%")
    print(f"     Yuqori shamlar: {pos_ret} ({pos_ret/(pos_ret+neg_ret)*100:.1f}%)")
    print(f"     Pastki shamlar: {neg_ret} ({neg_ret/(pos_ret+neg_ret)*100:.1f}%)")
    print(f"     Volatility/soat: {std_ret:.4f}%")
    print(f"  ➤ CGT uchun tavsiya:")
    if INTERVAL == "1h":
        thr_opt = std_ret * 0.8
        print(f"     SL threshold:   {thr_opt*2:.2f}% (2x volatility)")
        print(f"     TP threshold:   {thr_opt*5:.2f}% (5x volatility)")
        print(f"     Label threshold: {thr_opt:.3f}% (0.8x vol)")
    elif INTERVAL == "15m":
        thr_opt = std_ret * 0.6
        print(f"     SL threshold:   {thr_opt*3:.3f}%")
        print(f"     TP threshold:   {thr_opt*6:.3f}%")

# ════════════════════════════════════════════════════════════════
#  SAQLASH
# ════════════════════════════════════════════════════════════════
def save_csv(candles, filename):
    """CSV ga saqlash — CGT v3.0 formati"""
    with open(filename, "w", newline="") as f:
        writer = csv.writer(f)
        # Header — cgt_v3.cpp load_csv() bilan mos
        writer.writerow(["timestamp", "open", "high", "low", "close", "volume", "tf"])
        for c in candles:
            writer.writerow([
                c["ts"],
                f"{c['open']:.2f}",
                f"{c['high']:.2f}",
                f"{c['low']:.2f}",
                f"{c['close']:.2f}",
                f"{c['volume']:.4f}",
                INTERVAL,
            ])
    print(f"\n  💾 Saqlandi: {filename}")
    print(f"     Hajm: {os.path.getsize(filename)/1024:.1f} KB")

# ════════════════════════════════════════════════════════════════
#  ASOSIY
# ════════════════════════════════════════════════════════════════
def main():
    print("╔══════════════════════════════════════════════════════╗")
    print("║   CGT Data Fetcher v1.0                             ║")
    print("║   Binance Futures OHLCV yuklovchi + filtrlash       ║")
    print("╚══════════════════════════════════════════════════════╝")
    print(f"\n  Symbol:   {SYMBOL}")
    print(f"  Interval: {INTERVAL}")
    print(f"  Davr:     {DAYS} kun")
    print(f"  Chiqish:  {OUTPUT}")

    # Ulanish tekshirish
    print("\n[1] Binance ulanish tekshirilmoqda...")
    if not check_connection():
        print("[!] Binance ga ulanib bo'lmadi!")
        print("    Internet bor? VPN kerakmi?")
        sys.exit(1)
    print("    ✓ Binance Futures API — OK")

    # Server vaqti
    server_time = get_server_time()
    start_ms = server_time - (DAYS * 24 * 3600 * 1000)
    start_dt = datetime.fromtimestamp(start_ms/1000, tz=timezone.utc)
    print(f"    Boshlanish: {start_dt.strftime('%Y-%m-%d %H:%M UTC')}")

    # Yuklash
    print(f"\n[2] Ma'lumot yuklanmoqda...")
    print(f"    (Har 1000 sham = 1 so'rov, ~{DAYS*24//1000+1} ta so'rov)")

    all_raw = []
    current_ms = start_ms
    batch = 1000
    req_count = 0
    errors = 0

    while current_ms < server_time:
        raw = fetch_klines(SYMBOL, INTERVAL, current_ms, batch)

        if not raw:
            errors += 1
            if errors > 5:
                print(f"\n[!] Juda ko'p xato ({errors}), to'xtatildi")
                break
            print(f"\n[!] Bo'sh javob, 3 soniya kutilmoqda...")
            time.sleep(3)
            continue

        all_raw.extend(raw)
        req_count += 1
        errors = 0

        # Keyingi batch boshlash vaqti
        last_ts = int(raw[-1][0])
        current_ms = last_ts + iv_ms

        # Progress
        total_expected = DAYS * 24 * 3600 * 1000 // iv_ms
        pct = min(100, len(all_raw) / total_expected * 100)
        bar_len = 30
        filled = int(bar_len * pct / 100)
        bar = "█" * filled + "░" * (bar_len - filled)
        last_dt = datetime.fromtimestamp(last_ts/1000, tz=timezone.utc)
        print(f"\r    [{bar}] {pct:.1f}% | {len(all_raw):,} sham | {last_dt.strftime('%Y-%m-%d')} | req:{req_count}", end="", flush=True)

        # Rate limit: 2400 req/min → 25ms oraliq yetarli
        time.sleep(0.05)

        # Agar oxirgi batch to'liq bo'lmasa — tugadi
        if len(raw) < batch:
            break

    print(f"\n    ✓ Yuklandi: {len(all_raw):,} raw sham ({req_count} so'rov)")

    if not all_raw:
        print("[!] Hech qanday data yuklanmadi!")
        sys.exit(1)

    # Tozalash
    print(f"\n[3] Tozalash va filtrlash...")
    candles = clean_candles(all_raw)
    print(f"    Tozalash: {len(all_raw):,} → {len(candles):,}")
    candles = filter_candles(candles, verbose=True)

    # Sifat balli
    candles = add_quality_score(candles)
    good = sum(1 for c in candles if c.get("quality", 100) >= 70)
    print(f"     Sifatli (≥70): {good:,} ({good/len(candles)*100:.1f}%)")

    # Statistika
    print_statistics(candles)

    # Saqlash
    print(f"\n[4] CSV ga saqlash...")
    save_csv(candles, OUTPUT)

    # Qo'shimcha: sifatsiz shamlar hisoboti
    low_q = [c for c in candles if c.get("quality", 100) < 50]
    if low_q:
        print(f"\n  ⚠️  Past sifatli shamlar: {len(low_q)}")
        print(f"     (Model bu shamlarni ko'radi lekin og'irligi past)")

    print(f"""
╔══ TAYYOR ═══════════════════════════════════════════╗
║  Fayl:    {OUTPUT:<43}║
║  Shamlar: {len(candles):<43,}║
║  Keyingi: ./cgt --train                             ║
╚══════════════════════════════════════════════════════╝
""")

if __name__ == "__main__":
    main()
