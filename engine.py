import math

class Raqam:
    def __init__(self, data, _children=(), _op='', label=''):
        self.data = data
        self.grad = 0.0
        self._backward = lambda: None
        self._prev = set(_children)
        self._op = _op
        self.label = label

    def __repr__(self):
        return f"Raqam(data={self.data})"

    # 1. Qo'shish (+)
    def __add__(self, other):
        other = other if isinstance(other, Raqam) else Raqam(other)
        out = Raqam(self.data + other.data, (self, other), '+')
        
        def _backward():
            self.grad += 1.0 * out.grad
            other.grad += 1.0 * out.grad
        out._backward = _backward
        return out

    # 2. Ko'paytirish (*)
    def __mul__(self, other):
        other = other if isinstance(other, Raqam) else Raqam(other)
        out = Raqam(self.data * other.data, (self, other), '*')
        
        def _backward():
            self.grad += other.data * out.grad
            other.grad += self.data * out.grad
        out._backward = _backward
        return out
    
    # 3. Darajaga ko'tarish (**)
    def __pow__(self, other):
        assert isinstance(other, (int, float)), "Faqat int/float daraja mumkin"
        out = Raqam(self.data**other, (self,), f'**{other}')

        def _backward():
            self.grad += (other * self.data**(other-1)) * out.grad
        out._backward = _backward
        return out

    # --- YANGI QISM: MUAMMONI HAL QILADIGAN KODLAR ---
    
    # Teskari qo'shish: Agar (2 + Raqam) bo'lsa, uni (Raqam + 2) deb tushun
    def __radd__(self, other):
        return self + other

    # Teskari ko'paytirish: Agar (2 * Raqam) bo'lsa, uni (Raqam * 2) deb tushun
    def __rmul__(self, other):
        return self * other

    # Manfiy qilish (-Raqam)
    def __neg__(self): 
        return self * -1

    # Ayirish (-) -> (Raqam + (-Raqam))
    def __sub__(self, other): 
        return self + (-other)
    
    # Teskari ayirish (2 - Raqam)
    def __rsub__(self, other): 
        return other + (-self)

    # --------------------------------------------------

    def relu(self):
        out = Raqam(0 if self.data < 0 else self.data, (self,), 'ReLU')

        def _backward():
            self.grad += (out.data > 0) * out.grad
        out._backward = _backward
        return out

    def backward(self):
        topo = []
        visited = set()
        def build_topo(v):
            if v not in visited:
                visited.add(v)
                for child in v._prev:
                    build_topo(child)
                topo.append(v)
        build_topo(self)
        
        self.grad = 1.0
        for node in reversed(topo):
            node._backward()
    
    def tanh(self):
        x = self.data
        # Tanh formulasi: (e^(2x) - 1) / (e^(2x) + 1)
        t = (math.exp(2*x) - 1) / (math.exp(2*x) + 1)
        out = Raqam(t, (self,), 'tanh')

        def _backward():
            # Tanh hosilasi: 1 - tanh^2
            self.grad += (1 - t**2) * out.grad
        out._backward = _backward
        
        return out