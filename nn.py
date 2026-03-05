import random
from engine import Raqam

# 1. ODDIY NEYRON (Biz buni ko'rdik)
class Neuron:
    def __init__(self, nin):
        # nin = kiruvchi ma'lumotlar soni
        self.w = [Raqam(random.uniform(-1, 1)) for _ in range(nin)]
        self.b = Raqam(random.uniform(-1, 1))

    def __call__(self, x):
        act = sum((wi * xi for wi, xi in zip(self.w, x)), self.b)
        return act.tanh() # <--- Mana endi "Rembo" bo'ldi

    def parameters(self):
        return self.w + [self.b]

# 2. QATLAM (LAYER) - Bir nechta neyronlar guruhi
class Layer:
    def __init__(self, nin, nout):
        # nin = kiruvchi ma'lumotlar soni
        # nout = bu qatlamda nechta neyron bo'lishi
        self.neurons = [Neuron(nin) for _ in range(nout)]

    def __call__(self, x):
        outs = [n(x) for n in self.neurons]
        return outs[0] if len(outs) == 1 else outs

    def parameters(self):
        # Qatlamdagi barcha neyronlarning parametrlarini yig'ib beradi
        return [p for neuron in self.neurons for p in neuron.parameters()]

# 3. MLP (MULTI-LAYER PERCEPTRON) - Butun boshli tarmoq
class MLP:
    def __init__(self, nin, nouts):
        # nin = eng boshidagi kiruvchi ma'lumotlar soni
        # nouts = har bir qatlamda nechta neyron bo'lishi (ro'yxat)
        sz = [nin] + nouts
        self.layers = [Layer(sz[i], sz[i+1]) for i in range(len(nouts))]

    def __call__(self, x):
        for layer in self.layers:
            x = layer(x)
        return x

    def parameters(self):
        return [p for layer in self.layers for p in layer.parameters()]

# ---------------------------------------------------------
# SINAB KO'RISH (TEST)
# ---------------------------------------------------------
if __name__ == "__main__":
    # Keling, kichkina tarmoq tuzamiz:
    # Kirish (3 ta ma'lumot) -> O'rta qatlam (4 ta neyron) -> O'rta qatlam (4 ta neyron) -> Chiqish (1 ta javob)
    n = MLP(3, [4, 4, 1])

    # Kiruvchi ma'lumot (x)
    x = [Raqam(2.0), Raqam(3.0), Raqam(-1.0)]

    # Bashorat qilish
    natija = n(x)
    print("Tarmoq bashorati:", natija)

    # Orqaga qaytish (Backpropagation)
    natija.backward()
    print("Tarmoq ishladi va gradientlar hisoblandi!")
    
    # Parametrlar sonini sanaymiz
    print(f"Jami o'rgatiladigan parametrlar (w va b) soni: {len(n.parameters())} ta")