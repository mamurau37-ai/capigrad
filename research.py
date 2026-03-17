import pandas as pd
from sklearn.model_selection import train_test_split
from sklearn.ensemble import RandomForestClassifier
from sklearn.metrics import classification_report, accuracy_score

print("[*] Darslik (live_train_data.csv) yuklanmoqda...")
df = pd.read_csv("live_train_data.csv")

# Model o'rganadigan datchiklar
features = ['spread_bps', 'imb', 'ti60', 'cvd60', 'cls_up', 'cls_dn', 'cls_fl']

# 1. TOZALASH: Ketma-ket kelgan bir xil qatorlarni o'chirish (Haqiqiy mantiq)
# Bu yerda biz datchiklar o'zgargan qatorlarni aniqlab olamiz
mask = (df[features].shift() != df[features]).any(axis=1)
df_clean = df.loc[mask].dropna().copy()

print(f"[*] Original qatorlar: {len(df)}")
print(f"[*] Tozalashdan keyin: {len(df_clean)} (Egizaklar o'chirildi)")

X = df_clean[features]
y = df_clean['label']

# 2. VAQT BO'YICHA BO'LISH (Chronological Split)
# Ma'lumotni aralashtirmaymiz (shuffle=False), chunki bu vaqtli qatorli ma'lumot
X_train, X_test, y_train, y_test = train_test_split(X, y, test_size=0.2, shuffle=False)

print("[*] Sun'iy Intellekt (Random Forest) o'qitilmoqda...")
model = RandomForestClassifier(n_estimators=200, max_depth=12, min_samples_leaf=5, random_state=42, n_jobs=-1)
model.fit(X_train, y_train)

# 3. TEST
y_pred = model.predict(X_test)
acc = accuracy_score(y_test, y_pred)

print("\n" + "="*50)
print(f"🏆 REAL TEST ANIQLIGI (ACCURACY): {acc * 100:.2f}%")
print("="*50)
print("\nBatafsil Hisobot (Real bozor bashorati):")
print(classification_report(y_test, y_pred, target_names=['DOWN (-1)', 'FLAT (0)', 'UP (1)'], zero_division=0))

# Qaysi datchik eng muhim? (Feature Importance)
importances = pd.Series(model.feature_importances_, index=features).sort_values(ascending=False)
print("\n📊 Datchiklarning ahamiyati (Model kimga ko'proq ishonadi):")
print(importances)