#include <iostream>
#include <time.h>
#include "ThreadController.hpp"
#include "AlgoData.hpp"
#include "cpl_string.h"
#include "gdal_priv.h"
#include "cpl_conv.h" // for CPLMalloc()

void computeTileActual(int rasterX, int rasterY, int tileX, int tileY, int tileXo, int tileYo, int *tileXa, int *tileYa){
    *tileXa = rasterX - tileXo;
    *tileYa = rasterY - tileYo;

    *tileXa = (*tileXa > tileX)*tileX + !(*tileXa > tileX)* *tileXa;
    *tileYa = (*tileYa > tileY)*tileY + !(*tileYa > tileY)* *tileYa;
}

CPLErr invertColors(AlgoData data){
    CPLErr error;
    int i, size = data.tileXa*data.tileYa;
    error = data.inputBand->RasterIO( GF_Read, data.tileXo, data.tileYo, data.tileXa, data.tileYa,
                    data.buffer, data.tileXa, data.tileYa, GDT_Float32,
                    0, 0 );
    if(error){
        return error;
    }
    for(i=0; i<size; i++){
        data.buffer[i] = data.max - data.buffer[i];
    }

    error = data.outputBand->RasterIO( GF_Write, data.tileXo, data.tileYo, data.tileXa, data.tileYa,
                    data.buffer, data.tileXa, data.tileYa, GDT_Float32,
                    0, 0 );
    return error;
}

int main(){
    std::cout<<"Color inversion"<<std::endl;
    char filename[30] = "malbolgeEtalon_coh.tif";
    CPLErr error;
    AlgoData algoData;
    double adfGeoTransform[6];
    clock_t t = clock();

    algoData.min = 0.0;
    algoData.max=0.99;
    algoData.tileX = 1000;
    algoData.tileY = 1000;
    GDALAllRegister();

    const char *pszFormat = "GTiff";
    GDALDriver *poDriver;
    char **papszMetadata;
    poDriver = GetGDALDriverManager()->GetDriverByName(pszFormat);
    if( poDriver == NULL ){
        std::cerr<<"driver is null"<<std::endl;
        exit( 1 );
    }
    papszMetadata = poDriver->GetMetadata();
    if( !CSLFetchBoolean( papszMetadata, GDAL_DCAP_CREATE, FALSE ) ){
        std::cerr<<"driver does not support create()"<<std::endl;
        return 500;
    }

    GDALDataset *outputDataset;
    GDALDataset *inputDataset;
    char **outputOptions = NULL;

    inputDataset = (GDALDataset *) GDALOpen( filename, GA_ReadOnly );
    if(inputDataset == NULL){
        std::cerr<<"inputDataset can not be found"<<std::endl;
        return 404;
    }

    algoData.rasterX = inputDataset->GetRasterXSize();
    algoData.rasterY = inputDataset->GetRasterYSize();
    algoData.inputBand = inputDataset->GetRasterBand( 1 );
    if( inputDataset->GetGeoTransform( adfGeoTransform ) != CE_None ){
        std::cerr<<"geotransform corrupt. Can not continue";
        return 500;
    }
    algoData.inputBand = inputDataset->GetRasterBand(1);

    outputDataset = poDriver->Create( "inverted.tif", algoData.rasterX, algoData.rasterY, 1, GDT_Float32,
                                outputOptions );
    if(outputDataset == NULL){
        std::cerr<<"output dataset can not be formed"<<std::endl;
        return 500;
    }
    outputDataset->SetGeoTransform( adfGeoTransform );
    outputDataset->SetProjection(inputDataset->GetProjectionRef());
    algoData.outputBand = outputDataset->GetRasterBand(1);
    algoData.buffer = (float *) CPLMalloc(sizeof(float)*algoData.tileX*algoData.tileY);

    /*for(algoData.tileXo= 0; algoData.tileXo <algoData.rasterX; algoData.tileXo+= algoData.tileX){
        for(algoData.tileYo = 0; algoData.tileYo < algoData.rasterY; algoData.tileYo+= algoData.tileY){
            computeTileActual(algoData.rasterX, algoData.rasterY, algoData.tileX, algoData.tileY, algoData.tileXo, algoData.tileYo, &algoData.tileXa, &algoData.tileYa);
            std::cout<<algoData.tileXo << ")" << algoData.tileXa << " "<<algoData.tileYo << ")" << algoData.tileYa<<std::endl;
            error = invertColors(algoData);
            if(error){
        		std::cerr<<"gdal Error detected"<<std::endl;
                return 500;
        	}
        }
    }*/
    ThreadController controller(algoData);
    controller.startThreads();


    CPLFree(algoData.buffer);
    GDALClose((GDALDatasetH)outputDataset);
    GDALClose(inputDataset);
    t = clock() - t;
    std::cout<<"time taken: " << ((float)t)/CLOCKS_PER_SEC <<std::endl;
    return 0;
}
