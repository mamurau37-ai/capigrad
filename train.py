from engine import Raqam
from nn import MLP

# 1. MODELNI YARATAMIZ
# 3 ta kirish (input), ikkita 4 talik qavat, 1 ta chiqish (output)
model = MLP(3, [4, 4, 1])

# 2. MA'LUMOTLAR TO'PLAMI (DATASET)
xs = [
    [Raqam(2.0), Raqam(3.0), Raqam(-1.0)],
    [Raqam(3.0), Raqam(-1.0), Raqam(0.5)],
    [Raqam(0.5), Raqam(1.0), Raqam(1.0)],
    [Raqam(1.0), Raqam(1.0), Raqam(-1.0)],
]
ys = [Raqam(1.0), Raqam(-1.0), Raqam(-1.0), Raqam(1.0)]

# 3. O'QITISH HALQASI (TRAINING LOOP)
# Modelni 100 marta dars qildirib ko'ramiz
for k in range(100):
    
    # A) BASHORAT (Forward Pass)
    ypred = [model(x) for x in xs]
    
    # B) XATOLIKNI O'LCHASH (Loss Calculation)
    loss = sum((yout - ygt)**2 for ygt, yout in zip(ys, ypred))
    
    # C) NOLGA TUSHIRISH (Zero Grad)
    for p in model.parameters():
        p.grad = 0.0
    
    # D) ORQAGA QAYTISH (Backward Pass)
    loss.backward()
    
    # E) YANGILASH (Update / Optimization)
    learning_rate = 0.05 # Agar natija juda sekin o'zgarsa, buni 0.1 qilib ko'rish mumkin
    for p in model.parameters():
        p.data += -learning_rate * p.grad
        
    # Har 10 ta darsda bir natijani chiqaramiz
    if k % 10 == 0:
        print(f"Dars {k}: Xatolik (Loss) = {loss.data:.4f}")

print(f"Oxirgi xatolik: {loss.data:.4f}")

# 4. VIZUALIZATSIYA (VISUALIZATION)
print("\n--- CAPIGRAD MIYA XARITASI ---")
print("Qoida: Agar model '>' deb o'ylasa '#', '<' deb o'ylasa '.' chizadi.")
print("Bu modelning 'fikrlash chegarasi' hisoblanadi.\n")

# Y o'qi (tepadan pastga qarab chizamiz)
for y in range(20, -20, -2): 
    satr = ""
    # X o'qi (chapdan o'ngga)
    for x in range(-20, 20, 1):
        
        # Koordinatalarni kichraytiramiz (-2.0 dan 2.0 gacha bo'ladi)
        # Raqam klassi shu yerda ishlatilyapti, shuning uchun import eng tepada bo'lishi shart!
        x_k = Raqam(x / 10.0)
        y_k = Raqam(y / 10.0)
        z_k = Raqam(-1.0) # 3-chi ma'lumotni o'zgarmas deb oldik
        
        # Modelga beramiz
        inputs = [x_k, y_k, z_k]
        natija = model(inputs)
        
        # Agar natija 0 dan katta bo'lsa '#', kichik bo'lsa '.'
        if natija.data > 0:
            satr += "#" 
        else:
            satr += "."
            
    print(satr)