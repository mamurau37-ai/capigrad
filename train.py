from engine import Raqam
from nn import MLP

# 1. MODELNI YARATAMIZ
# 3 ta kirish (input), ikkita 4 talik qavat, 1 ta chiqish (output)
model = MLP(3, [4, 4, 1])

# 2. MA'LUMOTLAR TO'PLAMI (DATASET)
# Tasavvur qilaylik, bu "Imtihon javoblari":
# [Dars soati, Uyqu soati, O'yin soati] -> [Bahosi (1=Zo'r, -1=Yomon)]
xs = [
    [Raqam(2.0), Raqam(3.0), Raqam(-1.0)],
    [Raqam(3.0), Raqam(-1.0), Raqam(0.5)],
    [Raqam(0.5), Raqam(1.0), Raqam(1.0)],
    [Raqam(1.0), Raqam(1.0), Raqam(-1.0)],
]
# Biz kutayotgan to'g'ri javoblar (Labels)
ys = [Raqam(1.0), Raqam(-1.0), Raqam(-1.0), Raqam(1.0)]

# 3. O'QITISH HALQASI (TRAINING LOOP)
# Modelni 20 marta dars qildirib ko'ramiz
for k in range(100):
    
    # A) BASHORAT (Forward Pass)
    # Model har bir o'quvchi uchun o'z taxminini aytadi
    ypred = [model(x) for x in xs]
    
    # B) XATOLIKNI O'LCHASH (Loss Calculation)
    # Xato = (Bashorat - Asl Javob)^2
    # Bu "Mean Squared Error" deb ataladi
    # Xatolarni kvadratga ko'tarib yig'amiz
    loss = sum((yout - ygt)**2 for ygt, yout in zip(ys, ypred))    
    # C) NOLGA TUSHIRISH (Zero Grad)
    # Oldingi darsdagi xatolarni unutamiz (aks holda ular yig'ilib qoladi)
    for p in model.parameters():
        p.grad = 0.0
    
    # D) ORQAGA QAYTISH (Backward Pass)
    # Xato qayerdan kelganini topamiz (Gradientlarni hisoblaymiz)
    loss.backward()
    
    # E) YANGILASH (Update / Optimization)
    # Bu eng muhim qism! Parametrlarni to'g'ri tomonga salgina suramiz.
    learning_rate = 0.05
    for p in model.parameters():
        # p.grad - bu xatoning oshish yo'nalishi.
        # Biz xatoni kamaytirmoqchimiz, shuning uchun MINUS qilamiz.
        p.data += -learning_rate * p.grad
        
    print(f"Dars {k+1}: Xatolik (Loss) = {loss.data:.4f}")

# 4. TEKSHIRUV
print("\n--- O'qish tugadi! Natijalar: ---")
for x, y in zip(xs, ys):
    bashorat = model(x)
    print(f"Kutilgan: {y.data}, Model aytgani: {bashorat.data:.2f}")