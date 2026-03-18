#include "nn.hpp"
#include <cmath>
#include <random>
#include <algorithm>

// ── Aktivatsiyalar ───────────────────────────────────────────
double sigmoid_f(double x){ return 1.0/(1+std::exp(-std::max(-50.0,std::min(50.0,x)))); }
double dsigmoid_f(double x){ double s=sigmoid_f(x); return s*(1-s); }
double tanh_f(double x){ return std::tanh(x); }
double mish_f(double x){ double sp=std::log(1+std::exp(std::min(x,50.0))); return x*std::tanh(sp); }
double dmish_f(double x){ double sp=std::log(1+std::exp(std::min(x,50.0))),t=std::tanh(sp),s=sigmoid_f(x); return t+x*(1-t*t)*s; }
double swish_f(double x){ return x*sigmoid_f(x); }
double dswish_f(double x){ double s=sigmoid_f(x); return s*(1+x*(1-s)); }
double capi_f(double x){ return 0.5*mish_f(x)+0.5*swish_f(x); }
double dcapi_f(double x){ return 0.5*dmish_f(x)+0.5*dswish_f(x); }
double relu_f(double x){ return x>0?x:0.0; }
double clip(double x,double c){ return std::max(-c,std::min(c,x)); }

// ── Adam Optimizer ───────────────────────────────────────────
double AdamState::step(double g,double lr,int t,double b1,double b2,double eps,double wd){
    g=clip(g,3.0);
    m=b1*m+(1-b1)*g; v=b2*v+(1-b2)*g*g;
    double b1t=std::pow(b1,t), b2t=std::pow(b2,t);
    double mh=m/(1-b1t);
    double vh=v/(1-b2t);
    double update=lr*mh/(std::sqrt(vh)+eps);
    return update; 
}

double global_norm(const std::vector<double>& grads){
    double s=0; for(auto g:grads) s+=g*g; return std::sqrt(s);
}
void clip_by_norm(std::vector<double>& grads,double max_norm){
    double n=global_norm(grads);
    if(n>max_norm){ double s=max_norm/n; for(auto& g:grads) g*=s; }
}

// ── Neuron ────────────────────────────────────────────────────
Neuron::Neuron(int n,ActType a):act(a),w(n),dw(n,0),aw(n){
    std::mt19937 rng(std::random_device{}());
    double std_val;
    if(act==RELU_A||act==CAPI_A) std_val=std::sqrt(2.0/n);         
    else if(act==TANH_A||act==SIGMOID_A) std_val=std::sqrt(1.0/n); 
    else std_val=std::sqrt(0.1/n);                                  
    std::normal_distribution<double> d(0,std_val);
    for(auto& x:w) x=d(rng);
}
double Neuron::forward(const std::vector<double>& x){
    sum=b; for(int i=0;i<(int)w.size();i++) sum+=w[i]*x[i];
    switch(act){
        case CAPI_A:out=capi_f(sum);break; case TANH_A:out=tanh_f(sum);break;
        case RELU_A:out=relu_f(sum);break; case SIGMOID_A:out=sigmoid_f(sum);break;
        default:out=sum;
    }
    return out;
}
std::vector<double> Neuron::backward(const std::vector<double>& x,double delta){
    double da;
    switch(act){
        case CAPI_A:da=dcapi_f(sum);break; case TANH_A:da=1-tanh_f(sum)*tanh_f(sum);break;
        case RELU_A:da=sum>0?1.0:0.0;break; case SIGMOID_A:da=dsigmoid_f(sum);break;
        default:da=1.0;
    }
    double ds=clip(delta*da); db+=ds;
    std::vector<double> dx(x.size());
    for(int i=0;i<(int)w.size();i++){dw[i]+=ds*x[i]; dx[i]=ds*w[i];}
    return dx;
}
void Neuron::update(double lr,int t,double wd){
    for(int i=0;i<(int)w.size();i++){
        w[i]-=aw[i].step(dw[i],lr,t); 
        w[i]-=lr*wd*w[i];              
        dw[i]=0;
    }
    b-=ab.step(db,lr,t); db=0; 
}

// ── Layer ─────────────────────────────────────────────────────
Layer::Layer(int in,int out,ActType a){ for(int i=0;i<out;i++) ns.emplace_back(in,a); }
std::vector<double> Layer::forward(const std::vector<double>& x){
    last_in=x; std::vector<double> o; for(auto& n:ns) o.push_back(n.forward(x)); return o;
}
std::vector<double> Layer::backward(const std::vector<double>& d){
    std::vector<double> dx(last_in.size(),0);
    for(int i=0;i<(int)ns.size();i++){
        auto di=ns[i].backward(last_in,d[i]);
        for(int j=0;j<(int)di.size();j++) dx[j]+=di[j];
    }
    return dx;
}
void Layer::update(double lr,int t){ for(auto& n:ns) n.update(lr,t); }

// ── Temporal Layer (GRU) ──────────────────────────────────────
TemporalLayer::TemporalLayer(int in, int hid): in_sz(in), hid_sz(hid),
    Wr(hid, std::vector<double>(in+hid, 0)), br(hid, 0.0), dbr(hid, 0),
    dWr(hid, std::vector<double>(in+hid, 0)), Wr_adam(hid, std::vector<AdamState>(in+hid)), br_adam(hid),
    Wz(hid, std::vector<double>(in+hid, 0)), bz(hid, 0.0), dbz(hid, 0),
    dWz(hid, std::vector<double>(in+hid, 0)), Wz_adam(hid, std::vector<AdamState>(in+hid)), bz_adam(hid),
    Wu(hid, std::vector<double>(in+hid, 0)), bu(hid, 0.0), dbu(hid, 0),
    dWu(hid, std::vector<double>(in+hid, 0)), Wu_adam(hid, std::vector<AdamState>(in+hid)), bu_adam(hid),
    h(hid, 0), last_x(in, 0), last_h(hid, 0), last_r(hid, 0), last_z(hid, 0), last_u(hid, 0), last_su(hid, 0)
{
    std::mt19937 rng(std::random_device{}());
    double s = std::sqrt(1.0/(in+hid));
    std::normal_distribution<double> d(0, s);
    for(auto& row:Wr) for(auto& v:row) v=d(rng);
    for(auto& row:Wz) for(auto& v:row) v=d(rng);
    std::normal_distribution<double> dh(0, std::sqrt(2.0/(in+hid)));
    for(auto& row:Wu) for(auto& v:row) v=dh(rng);
    std::fill(bz.begin(), bz.end(), 1.0);
}

std::vector<double> TemporalLayer::forward(const std::vector<double>& x){
    last_x = x; last_h = h;
    std::vector<double> xh(in_sz+hid_sz);
    for(int i=0;i<in_sz;i++)   xh[i]       = x[i];
    for(int i=0;i<hid_sz;i++)  xh[in_sz+i] = h[i];

    for(int i=0;i<hid_sz;i++){
        double sr = br[i]; for(int j=0;j<in_sz+hid_sz;j++) sr += Wr[i][j]*xh[j]; last_r[i] = sigmoid_f(sr);
        double sz = bz[i]; for(int j=0;j<in_sz+hid_sz;j++) sz += Wz[i][j]*xh[j]; last_z[i] = sigmoid_f(sz);
        double su = bu[i];
        for(int j=0;j<in_sz;j++)    su += Wu[i][j]*x[j];
        for(int j=0;j<hid_sz;j++)   su += Wu[i][in_sz+j]*(last_r[i]*h[j]);
        last_su[i] = su; last_u[i]  = capi_f(su);
        h[i] = last_z[i]*h[i] + (1.0-last_z[i])*last_u[i];
    }
    return h;
}

std::vector<double> TemporalLayer::backward(const std::vector<double>& dh){
    std::vector<double> dx(in_sz, 0);
    std::vector<double> xh(in_sz+hid_sz);
    for(int i=0;i<in_sz;i++)   xh[i]       = last_x[i];
    for(int i=0;i<hid_sz;i++)  xh[in_sz+i] = last_h[i];

    for(int i=0;i<hid_sz;i++){
        double dhi = dh[i];
        double dz_val = clip(dhi * (last_h[i] - last_u[i]));
        double dz_sig = dz_val * last_z[i] * (1.0 - last_z[i]); 
        dbz[i] += dz_sig;
        for(int j=0;j<in_sz+hid_sz;j++){
            dWz[i][j] += dz_sig * xh[j];
            if(j < in_sz) dx[j] += dz_sig * Wz[i][j];
        }

        double du_val = clip(dhi * (1.0 - last_z[i]));
        double dsu    = du_val * dcapi_f(last_su[i]);
        dbu[i] += dsu;
        for(int j=0;j<in_sz;j++){ dWu[i][j] += dsu * last_x[j]; dx[j] += dsu * Wu[i][j]; }
        for(int j=0;j<hid_sz;j++) dWu[i][in_sz+j] += dsu * last_r[i] * last_h[j];

        double dr_sum = 0;
        for(int j=0;j<hid_sz;j++) dr_sum += dsu * Wu[i][in_sz+j] * last_h[j];
        double dr_sig = clip(dr_sum) * last_r[i] * (1.0 - last_r[i]);
        dbr[i] += dr_sig;
        for(int j=0;j<in_sz+hid_sz;j++){
            dWr[i][j] += dr_sig * xh[j];
            if(j < in_sz) dx[j] += dr_sig * Wr[i][j];
        }
    }
    return dx;
}

void TemporalLayer::update(double lr, int t, double wd){
    for(int i=0;i<hid_sz;i++){
        bz[i] -= bz_adam[i].step(dbz[i],lr,t); dbz[i]=0;
        for(int j=0;j<in_sz+hid_sz;j++){ Wz[i][j] -= Wz_adam[i][j].step(dWz[i][j],lr,t); Wz[i][j] -= lr*wd*Wz[i][j]; dWz[i][j]=0; }
        br[i] -= br_adam[i].step(dbr[i],lr,t); dbr[i]=0;
        for(int j=0;j<in_sz+hid_sz;j++){ Wr[i][j] -= Wr_adam[i][j].step(dWr[i][j],lr,t); Wr[i][j] -= lr*wd*Wr[i][j]; dWr[i][j]=0; }
        bu[i] -= bu_adam[i].step(dbu[i],lr,t); dbu[i]=0;
        for(int j=0;j<in_sz+hid_sz;j++){ Wu[i][j] -= Wu_adam[i][j].step(dWu[i][j],lr,t); Wu[i][j] -= lr*wd*Wu[i][j]; dWu[i][j]=0; }
    }
}

void TemporalLayer::reset(){ std::fill(h.begin(),h.end(),0); }
int TemporalLayer::pcount(){ return hid_sz*(in_sz+hid_sz)*6; } 

void TemporalLayer::save(std::ofstream& f){
    for(auto& row:Wu) for(auto& v:row) f.write((char*)&v,8);
    for(auto& row:Wr) for(auto& v:row) f.write((char*)&v,8);
    for(auto& row:Wz) for(auto& v:row) f.write((char*)&v,8);
    for(auto& v:bu) f.write((char*)&v,8);
    for(auto& v:br) f.write((char*)&v,8);
    for(auto& v:bz) f.write((char*)&v,8);
}
void TemporalLayer::load(std::ifstream& f){
    for(auto& row:Wu) for(auto& v:row) f.read((char*)&v,8);
    for(auto& row:Wr) for(auto& v:row) f.read((char*)&v,8);
    for(auto& row:Wz) for(auto& v:row) f.read((char*)&v,8);
    for(auto& v:bu) f.read((char*)&v,8);
    for(auto& v:br) f.read((char*)&v,8);
    for(auto& v:bz) f.read((char*)&v,8);
}
void Neuron::save(std::ofstream& f) {
    f.write((char*)&b, sizeof(b));
    for (double weight : w) {
        f.write((char*)&weight, sizeof(weight));
    }
}

void Neuron::load(std::ifstream& f) {
    f.read((char*)&b, sizeof(b));
    for (double& weight : w) {
        f.read((char*)&weight, sizeof(weight));
    }
}

void Layer::save(std::ofstream& f) {
    for (auto& n : ns) n.save(f);
}

void Layer::load(std::ifstream& f) {
    for (auto& n : ns) n.load(f);
}