import pandas as pd
from sklearn.model_selection import train_test_split
from sklearn.ensemble import RandomForestClassifier
from sklearn.metrics import classification_report, accuracy_score
import m2cgen as m2c

print("[*] Darslik (live_train_data.csv) yuklanmoqda...")
df = pd.read_csv("live_train_data.csv")

# Model o'rganadigan datchiklar (tartibi muhim! C++ ham shu tartibda beradi)
features = ['spread_bps', 'imb', 'ti60', 'cvd60', 'cls_up', 'cls_dn', 'cls_fl']

# Tozalash
mask = (df[features].shift() != df[features]).any(axis=1)
df_clean = df.loc[mask].dropna().copy()

X = df_clean[features]
y = df_clean['label']

X_train, X_test, y_train, y_test = train_test_split(X, y, test_size=0.2, shuffle=False)

print("[*] Sun'iy Intellekt o'qitilmoqda...")
# Tez ishlashi uchun va C++ da kod juda katta bo'lib ketmasligi uchun
# daraxtlar sonini (n_estimators) 50 ta qildik. Bu optimum.
model = RandomForestClassifier(n_estimators=50, max_depth=6, random_state=42, n_jobs=-1)
model.fit(X_train, y_train)

y_pred = model.predict(X_test)
acc = accuracy_score(y_test, y_pred)
print(f"🏆 REAL TEST ANIQLIGI (ACCURACY): {acc * 100:.2f}%")

print("\n[*] Jonli Maslahatchi (Random Forest) C++ kodiga aylantirilmoqda...")
# Python modelni sof C kodiga o'giramiz
cpp_code = m2c.export_to_c(model)

# C++ da funksiya nomi boshqalar bilan to'qnashmasligi uchun uni o'zgartiramiz
cpp_code = cpp_code.replace("void score(", "inline void live_rf_score(")

# Faylga yozamiz
with open("live_rf.hpp", "w") as f:
    f.write("#pragma once\n")
    f.write("#include <math.h>\n\n")
    f.write("// CapiGrad Live Advisor (Random Forest Auto-Generated)\n")
    f.write(cpp_code)

print("[+] Muvaffaqiyatli! 'live_rf.hpp' fayli yaratildi. Endi uni C++ ga ulash mumkin.")