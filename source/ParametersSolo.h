#ifndef CODE_ParametersSolo
#define CODE_ParametersSolo

#include <array>

#include "IncludeDefine.h"
#include "SoloBarcode.h"
#include "SoloFeatureTypes.h"

class Parameters;
class ParametersSolo;

class UMIdedup {
public:
    const static uint32 tN = 6;
    array<string,tN> typeNames { {"NoDedup", "Exact", "1MM_All", "1MM_Directional", "1MM_CR", "1MM_Directional_UMItools"} };
    enum typeI : int32 { NoDedup=0, Exact=1, All=2, Directional=3, CR=4, Directional_UMItools=5 };
    
    struct {
        uint32_t N;
        array<bool,tN> B;
        bool &NoDedup=B[0], &Exact=B[1], &All=B[2], &Directional=B[3], &CR=B[4], &Directional_UMItools=B[5]; 
    } yes;

    struct {
        //uint32_t N;
        array<uint32_t,tN> I;
        uint32_t &NoDedup=I[0], &Exact=I[1], &All=I[2], &Directional=I[3], &CR=I[4], &Directional_UMItools=I[5];
        uint32_t main; //index for SAM/stats/filtering output
    } countInd; //index in the countCellGennUMI
    
    vector<string> typesIn; //UMIdedup types from user options
    vector<int32> types; //the above converted to typeI numbers
    int32 typeMain; //the type to be used in SAM/stats/filtering output - for now just types[0]
    
    void initialize(ParametersSolo *pS);
    
protected:
    int it;
};

class ParametersSolo {
public:
    Parameters *pP;
    
    //chemistry, library etc
    string typeStr;
    enum SoloTypes : int32 {None=0, CB_UMI_Simple=1, CB_UMI_Complex=2, CB_samTagOut=3, SmartSeq=4};
    SoloTypes type;
    bool yes;
    string strandStr;
    int32 strand;   
    
    uint32 barcodeRead, barcodeReadIn;//which read is the barcode read = 0,1,2?
    uint32 barcodeStart, barcodeEnd;//start/end of barcode sequence on barcodeRead
    bool barcodeReadSeparate;
    
    //simple barcodes
    uint32 cbS, cbL; //cell barcode start,length
    uint32 umiS, umiL; //umi start,length
    uint32 bL, cbumiL; //total barcode sequene length, CB+UMI length. Former does may not be equal to the latter

    vector<string> cbPositionStr;
    string umiPositionStr;
    
    //complex barcodes    
    vector<SoloBarcode> cbV;
    SoloBarcode umiV; //single UMI
    bool adapterYes; //anchor?  
    string adapterSeq; //anchor sequence
    uint32 adapterMismatchesNmax;//max number of mismatches in the anchor
    
    //input from SAM files
    vector<string> samAtrrBarcodeSeq, samAtrrBarcodeQual;
    
    //whitelist - general
    uint64 cbWLsize;
    bool cbWLyes;
    vector<string> soloCBwhitelist;
    vector <uint64> cbWL;    
    vector<string> cbWLstr;
    
    //features
    vector<string> featureIn;//string of requested features
    vector<uint32> features;
    uint32 nFeatures;//=features.size(), number of requested features
    
    array<bool,SoloFeatureTypes::N> featureYes; //which features are requested
    array<bool,SoloFeatureTypes::N> readInfoYes;//which features will readInfo (for now only Gene)
    array<int32,SoloFeatureTypes::N> featureInd;//index of each feature - skips unrequested features
    
    //filtering
    char QSbase,QSmax;//quality score base and cutoff

    #ifdef MATCH_CellRanger
    double cbMinP;//for CBs with non-exact matching to WL, min posterior probability
    #else
    float cbMinP;//for CBs with non-exact matching to WL, min posterior probability
    #endif
    
    //cell filtering
    struct {
        vector<string> type;
        uint32 topCells;
        
        struct {
            double nExpectedCells;
            double maxPercentile;
            double maxMinRatio;
        } knee;
        
        struct {
            uint32 indMin, indMax; //min/max cell index, sorted by UMI counts,for empty cells
            uint32 umiMin;
            double umiMinFracMedian;
            uint32 candMaxN;
            double FDR;
            uint32 simN;
        } eDcr;//EmptyDrops-CellRanger
        
    } cellFilter;
      
    //CB match
    struct {
        string type;
        bool mm1; //1 mismatch allowed
        bool mm1_multi; //1 mismatch, multiple matches to WL allowed
        bool oneExact; //CBs require at least one exact match
        bool mm1_multi_pc; //use psedocounts while calculating probabilities of multi-matches
        bool mm1_multi_Nbase; //allow multimatching to WL for CBs with N-bases
    } CBmatchWL;
    
    //UMIdedup
    UMIdedup umiDedup;
    
    //multi-gene umi
    struct {
        vector<string> type;
        bool MultiGeneUMI;
        bool MultiGeneUMI_CR;
    } umiFiltering;
    
    //clusters
    string clusterCBfile;
    
    //output
    vector<string> outFileNames;    
    struct {
    	string featuresGeneField3;
    } outFormat;

    bool samAttrYes;//post-processed SAM attributes: error-corrected CB and UMI
    int32 samAttrFeature;//which feature to use for error correction

    //processing
    uint32 redistrReadsNfiles; //numer of files to resditribute reads into
    
    //constants
    uint32 umiMaskLow, umiMaskHigh; //low/high half bit-mask or UMIs

    void initialize(Parameters *pPin);
    void umiSwapHalves(uint32 &umi);
    void complexWLstrings();
    void cellFiltering();
};
#endif
