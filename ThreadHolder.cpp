#pragma once
#include "ThreadController.hpp"

ThreadHolder::ThreadHolder(AlgoData algoData, ThreadController *controller){
    this->algoData = algoData;
    this->controller = controller;
    std::thread worker(&ThreadHolder::work, this);
    worker.detach();
}

void ThreadHolder::work(){
    std::unique_lock<std::mutex> lk(this->mutex1);
    this->controller->getThreadSync()->wait(lk);

    CPLErr error = this->invertColors(this->algoData);

    this->controller->getOutMutex()->lock();
    if(error){
        std::cout<<"There was error" << error << ", but we do not care. Party continues! " << std::endl;
    }
    std::cout<< algoData.tileXo << ":" <<algoData.tileYo <<std::endl;
    this->controller->getOutMutex()->unlock();
    this->controller->registerThreadEnd();
}

CPLErr ThreadHolder::invertColors(AlgoData data){
    CPLErr error;
    int i, size = data.tileXa*data.tileYa;
    float *buffer = (float *) CPLMalloc(sizeof(float)*size);

    error = data.inputBand->RasterIO( GF_Read, data.tileXo, data.tileYo, data.tileXa, data.tileYa,
                    buffer, data.tileXa, data.tileYa, GDT_Float32,
                    0, 0 );
    if(error){
        CPLFree(buffer);
        return error;
    }
    for(i=0; i<size; i++){
        data.buffer[i] = data.max - data.buffer[i];
    }

    error = data.outputBand->RasterIO( GF_Write, data.tileXo, data.tileYo, data.tileXa, data.tileYa,
                    buffer, data.tileXa, data.tileYa, GDT_Float32,
                    0, 0 );
    CPLFree(buffer);
    return error;
}
