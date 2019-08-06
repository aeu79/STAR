#include "SoloReadFeature.h"
#include "serviceFuns.cpp"
#include "SequenceFuns.h"

uint32 outputReadCB(fstream *streamOut, int32 featureType, uint32 umiB, uint32 gene, vector<array<uint64,2>> &readSJs, const int32 cbMatch, const string &stringCB, const uint64 iRead, const vector<array<uint32,2>> &readTranscripts)
{   
    //format of the temp output file
    // UMI [iRead] type feature* stringCB
    //             0=exact match, 1=one non-exact match, 2=multipe non-exact matches
    //                   gene or sj[0] sj[1]
    //                           CB or nCB {CB Qual, ...}
    
    uint64 nout=1;
    if (featureType==-1) {//no feature, output for readInfo
        *streamOut << umiB <<' '<< iRead <<' '<< -1 <<' '<< cbMatch <<' '<< stringCB <<'\n';;
    } else if (featureType==0 || featureType==2) {//genes
        *streamOut << umiB <<' ';//UMI
        if ( iRead != (uint64)-1 )
            *streamOut << iRead <<' ';//iRead
        *streamOut << gene <<' '<< cbMatch <<' '<< stringCB <<'\n';;
    } else if (featureType==1) {//sjs
        for (auto &sj : readSJs) {
            *streamOut << umiB <<' ';//UMI
            if ( iRead != (uint64)-1 )
                *streamOut << iRead <<' ';//iRead            
            *streamOut << sj[0] <<' '<< sj[1] <<' '<< cbMatch <<' '<< stringCB <<'\n';;
        };
        nout=readSJs.size();
    } else if (featureType==3) {//transcript,distToTTS structure
        *streamOut << umiB <<' ';//UMI
        if ( iRead != (uint64)-1 )
            *streamOut << iRead <<' ';//iRead
        *streamOut << readTranscripts.size()<<' ';
        for (auto &tt: readTranscripts) {
            *streamOut << tt[0] <<' '<< tt[1] <<' ';
        };
        *streamOut << cbMatch <<' '<< stringCB <<'\n';
    };
    
    return nout;
};

void SoloReadFeature::record(SoloReadBarcode &soloBar, uint nTr, set<uint32> &readGene, set<uint32> &readGeneFull, Transcript *alignOut, uint64 iRead, const vector<array<uint32,2>> &readTranscripts)
{
    if (pSolo.type==0 || soloBar.cbMatch<0)
        return;

    vector<array<uint64,2>> readSJs;
    set<uint32> *readGe=&readGene; //for featureType==0

    bool readFeatYes=true;
    //calculate feature
    if (nTr==0) {//unmapped
        stats.V[stats.nUnmapped]++;
        readFeatYes=false;
    } else if (featureType==0 || featureType==2) {//genes
        //check genes, return if no gene of multimapping
        if (featureType==2) {
            readGe = &readGeneFull;
        };
        if (readGe->size()==0) {
            stats.V[stats.nNoFeature]++;
            readFeatYes=false;
        };
        if (readGe->size()>1) {
            stats.V[stats.nAmbigFeature]++;
            if (nTr>1)
                stats.V[stats.nAmbigFeatureMultimap]++;
            readFeatYes=false;
        };
    } else if (featureType==1) {//SJs
        if (nTr>1) {//reject all multimapping junctions
            stats.V[stats.nAmbigFeatureMultimap]++;
            readFeatYes=false;
        } else {//for SJs, still check genes, return if multi-gene
            if (readGene.size()>1) {
                stats.V[stats.nAmbigFeature]++;
                readFeatYes=false;
            } else {//one gene or no gene
                bool sjAnnot;
                alignOut->extractSpliceJunctions(readSJs, sjAnnot);
                if ( readSJs.empty() || (sjAnnot && readGene.size()==0) ) {//no junctions, or annotated junction but no gene (i.e. read does not fully match transcript)
                    stats.V[stats.nNoFeature]++;
                    readFeatYes=false;
                };
            };
        };
    } else if (featureType==3) {//transcripts
        if (readTranscripts.size()==0) {
            stats.V[stats.nNoFeature]++;
            readFeatYes=false;
        };
    };
    
    if (!readFeatYes && !readInfoYes) //no feature, and no readInfo requested
        return;
    
    if (!readInfoYes)
        iRead=(uint64)-1;

    uint32 nfeat = outputReadCB(streamReads, (readFeatYes ? featureType : -1), soloBar.umiB, *readGe->begin(), readSJs, soloBar.cbMatch, soloBar.cbMatchString, iRead, readTranscripts);
    if (pSolo.cbWLsize>0) {//WL
        for (auto &cbi : soloBar.cbMatchInd)
            cbReadCount[cbi] += nfeat;
    } else {//no WL
        cbReadCountMap[soloBar.cbMatchInd[0]] += nfeat;
    };
    
//     if (soloBar.cbMatch==0) {//exact match
//         uint32 n1 = outputReadCB(strU_0, featureType, soloBar.umiB, *readGe->begin(), readSJs, to_string(soloBar.cbI), iRead);
//         if (pSolo.cbWLsize>0) {//WL
//             cbReadCount[soloBar.cbI] += n1;
//         } else {//no WL
//             cbReadCountMap[soloBar.cbI] += n1;
//         };
//         return;
//     } else if (soloBar.cbMatch==1) {//1 match with 1MM
//         cbReadCount[soloBar.cbI]+= outputReadCB(strU_1, featureType, soloBar.umiB, *readGe->begin(), readSJs, to_string(soloBar.cbI), iRead);
//         return;
//     } else {//>1 matches
//         uint32 nfeat=outputReadCB(strU_2, featureType, soloBar.umiB, *readGe->begin(), readSJs, to_string(soloBar.cbMatch) + soloBar.cbMatchString, iRead);
//         for (auto &cbi : soloBar.cbMatchInd)
//             cbReadCount[cbi] += nfeat;
//         return;
//     };
};