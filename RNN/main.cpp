#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <cstdio>
#include <memory>
#include <armadillo>
#include <vector>
#include <math.h>

#include "RNN.h"

using namespace NeuralNet;

void workOnSequenceGeneration(std::shared_ptr<arma::mat> trainingY);
void testForward();
void trainRNN();
void testGrad();

int main(int argc, char *argv[]) {
//    testForward();
//    workOnSequenceGeneration();
    testGrad();
//    trainRNN();
    return 0;
}

// test the gradients by numerical gradients checking
void testGrad() {
    
    std::shared_ptr<arma::mat> trainingX(new arma::mat);
    std::shared_ptr<arma::mat> trainingY(new arma::mat);
    
    trainingX->randn(1, 10);
     trainingY->ones(1, 10);
    /* RNN constructor parameters passed as:
        RNN(int numHiddenLayers0, int hiddenLayerInputDim0,
        int hiddenLayerOutputDim0, int inputDim0, int outputDim0, 
        std::shared_ptr<arma::mat> trainingX0, std::shared_ptr<arma::mat> trainingY0)
     */
    RNN rnn(2, 2, 2, 1, 1, trainingX, trainingY);
    // before applying the LSTM backprop model, generate numerical gradients by just forward pass.
    rnn.calNumericGrad();
    
    rnn.forward();
    rnn.backward();

    
}

void workOnSequenceGeneration(std::shared_ptr<arma::mat> trainingY){
    // std::shared_ptr<arma::mat> trainingY(new arma::mat);
    // junhyukoh's data with only y labeled, no input x
    trainingY->load("testdata.dat",arma::raw_ascii);
    trainingY->print();
}


void testForward(){
    
    std::shared_ptr<arma::mat> trainingX(new arma::mat());
    trainingX->randn(1, 10);
    std::shared_ptr<arma::mat> trainingY(new arma::mat());
    trainingY->ones(1, 10);
//RNN::RNN_LSTM(int numHiddenLayers0, int hiddenLayerInputDim0,
//        int hiddenLayerOutputDim0, int inputDim0, int outputDim0, 
//        std::shared_ptr<arma::mat> trainingX0, std::shared_ptr<arma::mat> trainingY0)
    
    RNN rnn(1, 2, 2, 1, 1, trainingX, trainingY);
    rnn.forward();
    rnn.backward();

}

void trainRNN(){
    /*std::shared_ptr<arma::mat> trainingX(new arma::mat(1,10));
    
    trainingX->randn(1, 10);
    std::shared_ptr<arma::mat> trainingY(new arma::mat());
    trainingY->ones(3, 10);
     */
    
    std::shared_ptr<arma::mat> trainingX(new arma::mat());

    std::shared_ptr<arma::mat> trainingY(new arma::mat());
//    trainingY->load("testdata.dat");
//    *trainingY = arma::trans(*trainingY);
//    trainingX->zeros(trainingY->n_cols,1);
    

    
    trainingX->zeros(1, 10);
    trainingY->zeros(1,10);
    for (int i = 0; i <10; i++)
        trainingY->at(i) = i;
    trainingY->transform([](double val){return sin(val);});
    
    int iterations = 50000;
    
    /* RNN constructor parameters passed as:
        RNN(int numHiddenLayers0, int hiddenLayerInputDim0,
        int hiddenLayerOutputDim0, int inputDim0, int outputDim0, 
        std::shared_ptr<arma::mat> trainingX0, std::shared_ptr<arma::mat> trainingY0)
     */
    RNN rnn(2, 5, 5, 1, 1, trainingX, trainingY);
    // train the LSTM model by iterations
    for (int iter = 0; iter < iterations;iter++){
        rnn.train();
    }
    
    trainingY->save("trainingY.dat",arma::raw_ascii);
    for (int k = 0; k < 10; k++){
        (rnn.getOutputLayer()->outputMem[k])->print();
    }
}

