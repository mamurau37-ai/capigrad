import pandas as pd
import numpy as np
import os

LOG_FILE = "live_log.csv"
TRAIN_FILE = "live_train_data.csv"
FUTURE_TICKS = 60 
THRESHOLD_PCT = 0.05 

def process_live_logs():
    if not os.path.exists(LOG_FILE):
        print(f"[!] {LOG_FILE} topilmadi!")
        return

    print(f"[*] {LOG_FILE} o'qilmoqda...")
    df = pd.read_csv(LOG_FILE)
    df = df[df['row_type'] == 'TICK'].copy()

    if len(df) < FUTURE_TICKS:
        print("[!] Ma'lumot kam.")
        return

    # Kelajakni bashorat qilish uchun label yaratish
    df['future_price'] = df['price'].shift(-FUTURE_TICKS)
    df['target_return'] = (df['future_price'] - df['price']) / df['price'] * 100.0

    conditions = [(df['target_return'] > THRESHOLD_PCT), (df['target_return'] < -THRESHOLD_PCT)]
    choices = [1, -1]
    df['label'] = np.select(conditions, choices, default=0)
    df = df.dropna(subset=['future_price'])

    # 🚀 MANA SHU YERDA HAMMA USTUNLAR BO'LISHI SHART!
    features = ['ts', 'price', 'spread_bps', 'imb', 'ti60', 'cvd60', 'cls_up', 'cls_dn', 'cls_fl', 'label']
    
    # Faqat bizga kerakli ustunlarni saqlaymiz
    train_df = df[features]
    train_df.to_csv(TRAIN_FILE, index=False)
    
    print(f"[+] {TRAIN_FILE} muvaffaqiyatli yaratildi!")

if __name__ == "__main__":
    process_live_logs()