#include <cmath>
#include <random>
#include <assert.h>
#include "BaseLayer.h"

using namespace NeuralNet;

BaseLayer::BaseLayer(int inputDim0, int outputDim0, ActivationType actType0, 
        bool dropout, double dropr): inputDim(inputDim0),outputDim(outputDim0),
    actType(actType0), dropOutFlag(dropout), dropOutRate(dropr){
    initializeWeight();
    grad_W = std::make_shared<arma::mat>();
    grad_B = std::make_shared<arma::mat>();
    grad_W_accu = std::make_shared<arma::mat>();
    grad_B_accu = std::make_shared<arma::mat>();
    grad_W_accu->zeros(outputDim,inputDim);
    grad_B_accu->zeros(outputDim, 1);
    W_size = inputDim * outputDim;
    B_size = outputDim;
    totalSize = W_size + B_size;
    delta_out = std::make_shared<arma::mat>();
    output = std::make_shared<arma::mat>();
    
    if (dropOutFlag) {
        randomGen=new Random_Bernoulli<double>(dropOutRate);
    }
};



void BaseLayer::initializeWeight() {
    W.randu(outputDim,inputDim);
    B.randu(outputDim);
    W -= 0.5;
    B -= 0.5;

    if (actType == sigmoid) {
        (W) *=4*sqrt(6.0/(inputDim+outputDim));
        (B) *=4*sqrt(6.0/(inputDim+outputDim));
    } else if (actType == softmax) {
        (W) *=sqrt(6.0/(inputDim+outputDim));
        (B) *=sqrt(6.0/(inputDim+outputDim));

    }

}

void BaseLayer::save(std::string filename) {
    W.save(filename+"_W.dat",arma::raw_ascii);
    B.save(filename+"_B.dat",arma::raw_ascii);
}
void BaseLayer::load(std::string filename) {
    W.save(filename+"_W.dat",arma::raw_ascii);
    B.save(filename+"_B.dat",arma::raw_ascii);
}

void BaseLayer::updatePara(std::shared_ptr<arma::mat> delta_in, double learningRate) {
    calGrad(delta_in);
    B -= learningRate * (*grad_B);
    W -= learningRate * (*grad_W);
}

void BaseLayer::accumulateGrad(std::shared_ptr<arma::mat> delta_in){
    calGrad(delta_in);
    *grad_B_accu += *grad_B;
    *grad_W_accu += *grad_W;
}

void BaseLayer::updatePara_accu(double learningRate){
   
    B -= learningRate * (*grad_B_accu); 
    W -= learningRate * (*grad_W_accu);
}

void BaseLayer::calGrad(std::shared_ptr<arma::mat> delta_in){
    //for delta: each column is the delta of a sample
    arma::mat deriv;
    arma::mat delta;
 
    if (actType == softmax) {
        deriv.ones(output->n_rows,output->n_cols);   
    } else if (actType == sigmoid ) {
        deriv = (1 - (*output)) % (*output);        
    } else if ( actType == tanh) {
        deriv = (1 - (*output) % (*output));
    } else if ( actType == linear) {
        deriv.ones(output->n_rows,output->n_cols);
    }
    delta = (*delta_in) % deriv;
    *grad_B = arma::sum(delta,1);
    *grad_W = delta * (*input).st();

    if(dropOutFlag) {
        // for each column
       delta = delta % dropOutMat; 
    }
    (*delta_out) = W.st() * (delta);

}

void BaseLayer::applyActivation(){
// then do the activation
    std::shared_ptr<arma::mat> &p=output;
    arma::mat maxVal = arma::max(*p,0);
    arma::mat sumVal;
    switch(actType) {
    case softmax:
        for (int i = 0; i < p->n_cols; i++) {
                p->col(i)-= maxVal(i);            
        }          
        (*p).transform([](double val) {
            return exp(val);
        });

         sumVal = arma::sum(*p,0);
        for (int i = 0; i < p->n_cols; i++) {            
               p->col(i) /=sumVal(i);
        }
        break;
    case sigmoid:
//        p->print("p");
        (*p).transform([](double val) {
            return 1.0/(1.0+exp(-val));
        });
        break;
    case linear:
        break;
    }
}
void BaseLayer::activateUp(){
	assert((this->input!=NULL)&&"null ptr in the activateUp()");
	this->activateUp(this->input);
}
void BaseLayer::activateUp(std::shared_ptr<arma::mat> input0) {
    if(dropOutFlag){
//        BaseLayer::fill_Bernoulli(dropOutMat.memptr(),W_size);
    }
    
    
    input = input0;

    std::shared_ptr<arma::mat> &p=output;
// first get the projection
    if( dropOutFlag) {
    // for each column of the input
        *input = (*input) % dropOutMat;
    }
    
    (*output) = W * (*input);

    for (int i = 0; i < input->n_cols; i++) p->col(i) += B;

    applyActivation();
}
#if 0 
void BaseLayer::activateUp(std::shared_ptr<arma::mat> W_external, std::shared_ptr<arma::vec> B_external, std::shared_ptr<arma::mat> input0){
    input = input0;
    output = std::make_shared<arma::mat>(outputDim, input->n_cols);
    std::shared_ptr<arma::mat> &p=output;
// first get the projection
    (*output) = (*W_external) * (*input);

    for (int i = 0; i < input->n_cols; i++) p->col(i) += (*B_external);

    applyActivation();

}
#endif

void BaseLayer::vectoriseGrad(std::shared_ptr<arma::vec> V){
    
    *V = arma::vectorise(*grad_W);
    V->resize(W_size + B_size);
    V->rows(W_size,W_size+B_size-1) = *grad_B;
}

void BaseLayer::vectoriseWeight(std::shared_ptr<arma::vec> V){
    
    *V = arma::vectorise(W);
    V->resize(W_size + B_size);
    V->rows(W_size,W_size+B_size-1) = B;
}


void BaseLayer::deVectoriseWeight(std::shared_ptr<arma::vec> V){
    
    B =  V->rows(W_size,W_size+B_size-1);
    V->resize(W_size);
    W = *V;
    W.reshape(outputDim, inputDim);
}
// vectorise grad is frequency used to pass out the gradient as a vector
void BaseLayer::vectoriseGrad(double *ptr, size_t offset){

    double *W_ptr = grad_W->memptr();
    double *B_ptr = grad_B->memptr();
    for (int i = 0; i < W_size; i++){
        *(ptr + offset) = *(W_ptr+i);
        offset++;
    }
    for (int i = 0; i < B_size; i++){
        *(ptr + offset) = *(B_ptr+i);
        offset++;
    }

}

void BaseLayer::vectoriseWeight(double *ptr, size_t offset){
    
    double *W_ptr = W.memptr();
    double *B_ptr = B.memptr();
    for (int i = 0; i < W_size; i++){
        *(ptr + offset) = *(W_ptr+i);
        offset++;
    }
    for (int i = 0; i < B_size; i++){
        *(ptr + offset) = *(B_ptr+i);
        offset++;
    }
}

// devectorise weight is frequency used to pass out the gradient as a vector
void BaseLayer::deVectoriseWeight(double *ptr, size_t offset){
    
    double *W_ptr = W.memptr();
    double *B_ptr = B.memptr();
    for (int i = 0; i < W_size; i++){
        *(W_ptr+i) = *(ptr + offset) ;
        offset++;
    }
    for (int i = 0; i < B_size; i++){
        *(B_ptr+i) = *(ptr + offset) ;
        offset++;
    }
}

void BaseLayer::fill_Bernoulli(double *p, int size){

    for (int i = 0; i < size; i++){
        if(randomGen->next()) *(p+i) = 1.0;
        else *(p+i) = 0.0;
    }
}
