import random
import math
from engine import Raqam
from nn import MLP

# ---------------------------------------------------------
# 1. MA'LUMOTLARNI TAYYORLASH (DATA GENERATION)
# ---------------------------------------------------------
print("Ma'lumotlar yaratilmoqda...", end=" ")

xs = []
ys = []

# 100 ta nuqta yaratamiz
for _ in range(100):
    # -2.0 va 2.0 oralig'ida tasodifiy nuqta (x, y)
    x = random.uniform(-2.0, 2.0)
    y = random.uniform(-2.0, 2.0)
    
    # Keling, matematik doira chizamiz
    # Formula: x^2 + y^2 <= 1 (Radius 1 bo'lgan doira)
    # Agar nuqta markazga yaqin bo'lsa (1), uzoq bo'lsa (-1)
    label = 1.0 if (x*x + y*y) <= 1.0 else -1.0
    
    # Modelga beriladigan ko'rinishga keltiramiz
    xs.append([Raqam(x), Raqam(y)])
    ys.append(Raqam(label))

print("Tayyor! (100 ta nuqta)")

# ---------------------------------------------------------
# 2. MODELNI KUCHAYTIRISH (BIGGER BRAIN)
# ---------------------------------------------------------
# Oldin: [4, 4, 1] edi. 
# Hozir: [16, 16, 1] qilamiz. Neyronlar sonini 4 barobar oshiramiz!
# Kirish endi 2 ta (x va y), 3 ta emas.
model = MLP(2, [16, 16, 1]) 
print("Model kuchaytirildi: 16 talik neyron qavatlari o'rnatildi.")

# ---------------------------------------------------------
# 3. PROFESSIONAL O'QITISH (TRAINING)
# ---------------------------------------------------------
# ... (kodning boshi o'sha-o'sha) ...

# 3. PROFESSIONAL O'QITISH (TRAINING)
print("O'qitish boshlandi... (Sabr qilasiz, bu og'ir operatsiya)")

# Dars sonini 200 ga ko'tarib, sabrli bo'lamiz
for k in range(200):
    
    ypred = [model(x) for x in xs]
    
    # Xatolikni hisoblash
    total_loss = sum((yout - ygt)**2 for ygt, yout in zip(ys, ypred))
    
    # Gradientlarni tozalash
    for p in model.parameters():
        p.grad = 0.0
    
    # Orqaga qaytish
    total_loss.backward()
    
    # YANGILASH: Tezlikni (Learning Rate) 0.1 da ushlab turamiz.
    # Bu modelni "sakrab ketmasligini" ta'minlaydi.
    learning_rate = 0.00002 
    
    for p in model.parameters():
        p.data += -learning_rate * p.grad
    
    # Har 10 ta darsda natijani ko'rsatamiz
    if k % 10 == 0:
        print(f"Dars {k}/200 | Xatolik: {total_loss.data:.4f}")

# ... (Vizualizatsiya qismi o'sha-o'sha) ...
# ---------------------------------------------------------
# 4. VIZUALIZATSIYA (ASCII ART 2.0)
# ---------------------------------------------------------
print("\n--- DOIRA TESTI ---")
for y in range(20, -20, -2): 
    satr = ""
    for x in range(-20, 20, 1):
        x_k = Raqam(x / 10.0)
        y_k = Raqam(y / 10.0)
        
        natija = model([x_k, y_k])
        
        if natija.data > 0:
            satr += "O" # Ichkarini 'O' qilamiz
        else:
            satr += "." # Tashqarini '.' qilamiz
            
    print(satr)