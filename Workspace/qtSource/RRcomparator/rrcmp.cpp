#include "rrcmp.h"
#include "ui_rrcmp.h"
#include <iostream>
#include <string>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <QFile>
#include <QCheckBox>
#include <QTextStream>
#include <QDir>
#include <qwt_plot.h>
#include <qwt_plot_curve.h>

using namespace std;
using namespace cv;

//sampling matrix
const int Nj=181;
const int Ni=171;
const int Nptn=5;//N. partners
const int Npar=6;//N. parameters + media
//sampling matrix
double S[Nj][Ni][Npar][Nptn+2][2];
//      [j][i][0][k][m] = z (mm)
//      [j][i][1][k][m] = dz/dx=Dx
//      [j][i][2][k][m] = dz/dy=Dy
//      [j][i][3][k][m] = devZ (mm)
//      [j][i][4][k][m] = devDx
//      [j][i][5][k][m] = devDy
//      [j][i][l][0][m] -> ENEA
//      [j][i][l][1][m] -> FISE
//      [j][i][l][2][m] -> DLR
//      [j][i][l][3][m] -> NREL
//      [j][i][l][4][m] -> SANDIA
//      [j][i][l][5][m] -> mean
//      [j][i][l][6][m] -> ideal
//      [j][i][l][k][0] = N. of input data
//      [j][i][l][k][1] = mean value
int Nloaded=0;

//attaching points
//  P3         P4
//             ^ x
//             |
//  P1  y<-----P2
int iP2=40;//cell centered in P2
int jP2=36;
int iP3,jP1;
int iMin=Ni,iMax=0,jMin=Nj,jMax=0;
double DS=10.0;//cell step
double range[Npar][Nptn+1][5];
//          [iPar][iPtn][0] min
//          [iPar][iPtn][1] max
//          [iPar][iPtn][2] mean
//          [iPar][iPtn][3] RMS
//          [iPar][iPtn][4] Peak-Valley
double vP2[3],vP4[3];//ideal values at the attaching points P2 and P4
//        [0] z
//        [1] Dx
//        [2] Dy
double Vservice[3];

QString dir;
QString partner[Nptn+2]={"ENEA","FISE","DLR","NREL","SANDIA","Mean","Ideal"};
QString partnerChar[Nptn]={"E","F","D","N","S"};
QString param[Npar]={"z","Dx","Dy","devZ","devDx","devDy"};
QString specim[6]={"inner#60","inner#61","inner#62","outer#93","outer#97","outer#99"};

QwtPlot *G1;
int nOpenGraph=0;//number of plots

//invoked functions
int NINT(double x);
void pxColor(double val);

RRcmp::RRcmp(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::RRcmp)
{
    ui->setupUi(this);

    printf("****************************************************\n");
    printf("              Program C++ RRcomparator\n\n");
    printf("\tSoftware to compare the results got in\n\tthe SFERA-III WP10 3D-shape round-robin \n");
    printf("         version 7.0 14 February 2024\n\n");
    printf("       Main author: Marco Montecchi, ENEA (Italy)\n");
    printf("          email: marco.montecchi@enea.it\n");
    printf("          Porting to Windows by\n");
    printf("               Alberto Mittiga, ENEA (Italy)\n");
    printf("          email: alberto.mittiga@enea.it\n ");
    printf("****************************************************\n");


    //signals and slots
    connect(ui->checkBox_0, SIGNAL(stateChanged(int)),this,SLOT(loadENEA()));
    connect(ui->checkBox_1, SIGNAL(stateChanged(int)),this,SLOT(loadFISE()));
    connect(ui->checkBox_2, SIGNAL(stateChanged(int)),this,SLOT(loadDLR()));
    connect(ui->checkBox_3, SIGNAL(stateChanged(int)),this,SLOT(loadNREL()));
    connect(ui->checkBox_4, SIGNAL(stateChanged(int)),this,SLOT(loadSANDIA()));
    connect(ui->checkBox_showDiff, SIGNAL(stateChanged(int)),this,SLOT(displayAP()));
    connect(ui->comboBox_SWrealignment, SIGNAL(currentIndexChanged(int)),this,SLOT(loadAll()));
    connect(ui->comboBox_spec, SIGNAL(currentIndexChanged(int)),this,SLOT(loadAll()));
    connect(ui->comboBox_par, SIGNAL(currentIndexChanged(int)),this,SLOT(displayAP()));
    connect(ui->spinBox_decimal,SIGNAL(valueChanged(int)),this,SLOT(displayAP()));
    connect(ui->checkBox_mrad,SIGNAL(stateChanged(int)),this,SLOT(displayAP()));
    connect(ui->checkBox_limComXY,SIGNAL(stateChanged(int)),this,SLOT(checkRange()));
    connect(ui->pushButton_compare, SIGNAL(pressed()),this,SLOT(compare()));
    connect(ui->pushButton_map_0, SIGNAL(pressed()),this,SLOT(map0()));
    connect(ui->pushButton_map_1, SIGNAL(pressed()),this,SLOT(map1()));
    connect(ui->pushButton_map_2, SIGNAL(pressed()),this,SLOT(map2()));
    connect(ui->pushButton_map_3, SIGNAL(pressed()),this,SLOT(map3()));
    connect(ui->pushButton_map_4, SIGNAL(pressed()),this,SLOT(map4()));

    idToLineEdit["lineEdit_EP1"]=ui->lineEdit_EP1;
    idToLineEdit["lineEdit_EP2"]=ui->lineEdit_EP2;
    idToLineEdit["lineEdit_EP3"]=ui->lineEdit_EP3;
    idToLineEdit["lineEdit_EP4"]=ui->lineEdit_EP4;
    idToLineEdit["lineEdit_FP1"]=ui->lineEdit_FP1;
    idToLineEdit["lineEdit_FP2"]=ui->lineEdit_FP2;
    idToLineEdit["lineEdit_FP3"]=ui->lineEdit_FP3;
    idToLineEdit["lineEdit_FP4"]=ui->lineEdit_FP4;
    idToLineEdit["lineEdit_DP1"]=ui->lineEdit_DP1;
    idToLineEdit["lineEdit_DP2"]=ui->lineEdit_DP2;
    idToLineEdit["lineEdit_DP3"]=ui->lineEdit_DP3;
    idToLineEdit["lineEdit_DP4"]=ui->lineEdit_DP4;
    idToLineEdit["lineEdit_NP1"]=ui->lineEdit_NP1;
    idToLineEdit["lineEdit_NP2"]=ui->lineEdit_NP2;
    idToLineEdit["lineEdit_NP3"]=ui->lineEdit_NP3;
    idToLineEdit["lineEdit_NP4"]=ui->lineEdit_NP4;
    idToLineEdit["lineEdit_SP1"]=ui->lineEdit_SP1;
    idToLineEdit["lineEdit_SP2"]=ui->lineEdit_SP2;
    idToLineEdit["lineEdit_SP3"]=ui->lineEdit_SP3;
    idToLineEdit["lineEdit_SP4"]=ui->lineEdit_SP4;

    idToLineEdit["lineEdit_Emin"]=ui->lineEdit_Emin;
    idToLineEdit["lineEdit_Fmin"]=ui->lineEdit_Fmin;
    idToLineEdit["lineEdit_Dmin"]=ui->lineEdit_Dmin;
    idToLineEdit["lineEdit_Nmin"]=ui->lineEdit_Nmin;
    idToLineEdit["lineEdit_Smin"]=ui->lineEdit_Smin;
    idToLineEdit["lineEdit_Emax"]=ui->lineEdit_Emax;
    idToLineEdit["lineEdit_Fmax"]=ui->lineEdit_Fmax;
    idToLineEdit["lineEdit_Dmax"]=ui->lineEdit_Dmax;
    idToLineEdit["lineEdit_Nmax"]=ui->lineEdit_Nmax;
    idToLineEdit["lineEdit_Smax"]=ui->lineEdit_Smax;

    idToLineEdit["lineEdit_ExMin"]=ui->lineEdit_ExMin;
    idToLineEdit["lineEdit_FxMin"]=ui->lineEdit_FxMin;
    idToLineEdit["lineEdit_DxMin"]=ui->lineEdit_DxMin;
    idToLineEdit["lineEdit_NxMin"]=ui->lineEdit_NxMin;
    idToLineEdit["lineEdit_SxMin"]=ui->lineEdit_SxMin;
    idToLineEdit["lineEdit_ExMax"]=ui->lineEdit_ExMax;
    idToLineEdit["lineEdit_FxMax"]=ui->lineEdit_FxMax;
    idToLineEdit["lineEdit_DxMax"]=ui->lineEdit_DxMax;
    idToLineEdit["lineEdit_NxMax"]=ui->lineEdit_NxMax;
    idToLineEdit["lineEdit_SxMax"]=ui->lineEdit_SxMax;

    idToLineEdit["lineEdit_EyMin"]=ui->lineEdit_EyMin;
    idToLineEdit["lineEdit_FyMin"]=ui->lineEdit_FyMin;
    idToLineEdit["lineEdit_DyMin"]=ui->lineEdit_DyMin;
    idToLineEdit["lineEdit_NyMin"]=ui->lineEdit_NyMin;
    idToLineEdit["lineEdit_SyMin"]=ui->lineEdit_SyMin;
    idToLineEdit["lineEdit_EyMax"]=ui->lineEdit_EyMax;
    idToLineEdit["lineEdit_FyMax"]=ui->lineEdit_FyMax;
    idToLineEdit["lineEdit_DyMax"]=ui->lineEdit_DyMax;
    idToLineEdit["lineEdit_NyMax"]=ui->lineEdit_NyMax;
    idToLineEdit["lineEdit_SyMax"]=ui->lineEdit_SyMax;

    idToLineEdit["lineEdit_E_Mean"]=ui->lineEdit_E_Mean;
    idToLineEdit["lineEdit_F_Mean"]=ui->lineEdit_F_Mean;
    idToLineEdit["lineEdit_D_Mean"]=ui->lineEdit_D_Mean;
    idToLineEdit["lineEdit_N_Mean"]=ui->lineEdit_N_Mean;
    idToLineEdit["lineEdit_S_Mean"]=ui->lineEdit_S_Mean;

    idToLineEdit["lineEdit_E_RMS"]=ui->lineEdit_E_RMS;
    idToLineEdit["lineEdit_F_RMS"]=ui->lineEdit_F_RMS;
    idToLineEdit["lineEdit_D_RMS"]=ui->lineEdit_D_RMS;
    idToLineEdit["lineEdit_N_RMS"]=ui->lineEdit_N_RMS;
    idToLineEdit["lineEdit_S_RMS"]=ui->lineEdit_S_RMS;

    idToLineEdit["lineEdit_E_PV"]=ui->lineEdit_E_PV;
    idToLineEdit["lineEdit_F_PV"]=ui->lineEdit_F_PV;
    idToLineEdit["lineEdit_D_PV"]=ui->lineEdit_D_PV;
    idToLineEdit["lineEdit_N_PV"]=ui->lineEdit_N_PV;
    idToLineEdit["lineEdit_S_PV"]=ui->lineEdit_S_PV;

    idToLineEdit["lineEdit_E_Min"]=ui->lineEdit_E_Min;
    idToLineEdit["lineEdit_F_Min"]=ui->lineEdit_F_Min;
    idToLineEdit["lineEdit_D_Min"]=ui->lineEdit_D_Min;
    idToLineEdit["lineEdit_N_Min"]=ui->lineEdit_N_Min;
    idToLineEdit["lineEdit_S_Min"]=ui->lineEdit_S_Min;

    idToLineEdit["lineEdit_E_Max"]=ui->lineEdit_E_Max;
    idToLineEdit["lineEdit_F_Max"]=ui->lineEdit_F_Max;
    idToLineEdit["lineEdit_D_Max"]=ui->lineEdit_D_Max;
    idToLineEdit["lineEdit_N_Max"]=ui->lineEdit_N_Max;
    idToLineEdit["lineEdit_S_Max"]=ui->lineEdit_S_Max;

    idToCheckBox["checkBox_0"]=ui->checkBox_0;
    idToCheckBox["checkBox_1"]=ui->checkBox_1;
    idToCheckBox["checkBox_2"]=ui->checkBox_2;
    idToCheckBox["checkBox_3"]=ui->checkBox_3;
    idToCheckBox["checkBox_4"]=ui->checkBox_4;

    // parameter initialization
#ifdef __unix__
#define IS_POSIX 1
#else
#define IS_POSIX 0
#endif
    QDir dir2;  //current directory
    dir2.cdUp();//cd ..
    dir2.cdUp();//cd ..
    if (IS_POSIX == 1) {
        //Linux path initialization
        //nothing to do
    }
    else {
        //windows path inizialization
        dir2.cdUp();//cd ..
    }
    dir=dir2.absolutePath()+"/RRs3wp10/";
    printf("dir= %s\n",dir.toStdString().c_str());

    //reset common range
    for(int iP=0;iP<Npar;iP++){
        range[iP][5][0]=1.e+06;
        range[iP][5][1]=-1.e+06;
    }
}

RRcmp::~RRcmp()
{
    delete ui;
}

void RRcmp::loadAll(){
    idealValue();
    Qt::CheckState state=ui->checkBox_0 -> checkState();
    if( state == Qt::Checked )
        loadData(0);
    state=ui->checkBox_1 -> checkState();
    if( state == Qt::Checked )
        loadData(1);
    state=ui->checkBox_2 -> checkState();
    if( state == Qt::Checked )
        loadData(2);
    state=ui->checkBox_3 -> checkState();
    if( state == Qt::Checked )
        loadData(3);
    state=ui->checkBox_4 -> checkState();
    if( state == Qt::Checked )
        loadData(4);
    checkRange();
}

void RRcmp::loadENEA(){
    Qt::CheckState state=ui->checkBox_0 -> checkState();
    if( state != Qt::Checked ){
        checkRange();
        return;
    }
    idealValue();
    loadData(0);
    checkRange();
}

void RRcmp::loadFISE(){
    Qt::CheckState state=ui->checkBox_1 -> checkState();
    if( state != Qt::Checked ){
        checkRange();
        return;
    }
    idealValue();
    loadData(1);
    checkRange();
}

void RRcmp::loadDLR(){
    Qt::CheckState state=ui->checkBox_2 -> checkState();
    if( state != Qt::Checked ){
        checkRange();
        return;
    }
    idealValue();
    loadData(2);
    checkRange();
}

void RRcmp::loadNREL(){
    Qt::CheckState state=ui->checkBox_3 -> checkState();
    if( state != Qt::Checked ){
        checkRange();
        return;
    }
    idealValue();
    loadData(3);
    checkRange();
}

void RRcmp::loadSANDIA(){
    Qt::CheckState state=ui->checkBox_4 -> checkState();
    if( state != Qt::Checked ){
        checkRange();
        return;
    }
    idealValue();
    loadData(4);
    checkRange();
}

void RRcmp::loadData(int iPtn){
    printf("->loadData for partner=%d",iPtn);
    int iSpec=ui->comboBox_spec->currentIndex();
    QString fileName=dir+partner[iPtn]+"_"+specim[iSpec]+".dat";
    cout << " fileName= "<<fileName.toStdString()<<"  ....";
    fflush(stdout);
    QFile file(fileName);
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text)){
        printf("file %s non trovato!!!\n",fileName.toStdString().c_str());
        idToCheckBox["checkBox_"+QString::number(iPtn)]->setCheckState(Qt::Unchecked);
        return;
    }
    //S matrix initialization for iPtn
    for(int i=0;i<Ni;i++){
        for(int j=0;j<Nj;j++){
            for(int iP=0;iP<Npar;iP++){
                S[j][i][iP][iPtn][0]=0.;
                S[j][i][iP][iPtn][1]=0.;
            }
        }
    }
    QTextStream stream(&file);
    QString line,pezzo;
    QStringList List;
    int iC=0;
    double x,y,val;
    //read data
    do{
        line=stream.readLine();
        line=line.simplified();
        if(line.contains("nan"))
            continue;
        if(line.contains("mm"))
            continue;
        List =line.split(" ");
        int nV=List.count();
        iC++;
        if(nV!=8){
            printf("\nERROR: data file must consist of 8 column; @iC=%d found %d\n",iC,nV);
            printf("skipped line %s\n",line.toStdString().c_str());
            continue;
        }
        pezzo=List.at(0).toLocal8Bit().constData();
        x=pezzo.toDouble();
        pezzo=List.at(1).toLocal8Bit().constData();
        y=pezzo.toDouble();
        int i=NINT(x/DS)+iP2;
        int j=NINT(y/DS)+jP2;
        if(i<0 || j<0 || i>=Ni || j>=Nj){
            printf("Error: i=%d j=%d %s\n",i,j,line.toStdString().c_str());
            continue;
        }
        for(int iv=2;iv<nV;iv++){
            pezzo=List.at(iv).toLocal8Bit().constData();
            val=pezzo.toDouble();
            S[j][i][iv-2][iPtn][1]=S[j][i][iv-2][iPtn][1]+val;
            S[j][i][iv-2][iPtn][0]++;
        }
    }while(!stream.atEnd());
    file.close();
    //reset range
    for(int iP=0;iP<Npar;iP++){
        range[iP][iPtn][0]=1.e+06;
        range[iP][iPtn][1]=-1.e+06;
    }
    printf("done!\n");
    fflush(stdout);
    //S matrix normalization to Ndata per cell
    //int Imin=Ni,Imax=0,Jmin=Nj,Jmax=0;
    for(int i=0;i<Ni;i++){
        for(int j=0;j<Nj;j++){
            for(int iP=0;iP<Npar;iP++){
                if(S[j][i][iP][iPtn][0]>0.5){
                    S[j][i][iP][iPtn][1]=S[j][i][iP][iPtn][1]/S[j][i][iP][iPtn][0];
                    if(iPtn==1 || iPtn==2){
                        if(iP==1)
                            S[j][i][iP][iPtn][1]=tan(S[j][i][iP][iPtn][1]);//FISE & DLR consider theta(rad) and not tan(theta)
                        else if(iP==4)
                            S[j][i][iP][iPtn][1]=tan(atan(S[j][i][1][iPtn][1])+S[j][i][iP][iPtn][1])-S[j][i][1][iPtn][1];
                    }
                }
            }
        }
    }

    //software realignment to set z@PJ=z_ideal
    int iSWr=ui->comboBox_SWrealignment->currentIndex() ;
    if(iSWr!=0){
        double mR,mL,mB,mT,my,mx,corZ,dZ1,dZ2,dZ3,dZ4;
        if(iSWr==1){
            dZ1=S[jP1][iP2][0][iPtn][1]-vP2[0];
            dZ2=S[jP2][iP2][0][iPtn][1]-vP2[0];
            dZ3=S[jP1][iP3][0][iPtn][1]-vP4[0];
            dZ4=S[jP2][iP3][0][iPtn][1]-vP4[0];
        }
        else{
            dZ1=S[jP1][iP2][3][iPtn][1];
            dZ2=S[jP2][iP2][3][iPtn][1];
            dZ3=S[jP1][iP3][3][iPtn][1];
            dZ4=S[jP2][iP3][3][iPtn][1];
        }
        printf("dZ3=%e\tdZ4=%e\ndZ1=%e\tdZ2=%e\n",dZ3,dZ4,dZ1,dZ2);
        mR=(dZ4-dZ2)/(iP3-iP2)/DS;//right  P2->P4
        mL=(dZ3-dZ1)/(iP3-iP2)/DS;//left   P1->P3
        mB=(dZ1-dZ2)/(jP1-jP2)/DS;//bottom P2->P1
        mT=(dZ3-dZ4)/(jP1-jP2)/DS;//top    P4->P3
        printf("\tmT=%e\nmL=%e\tmR=%e\n\tmB=%e\n",mT,mL,mR,mB);
        for(int i=0;i<Ni;i++){
            double dZR=dZ2+mR*(i-iP2)*DS;
            double dZL=dZ1+mL*(i-iP2)*DS;
            my=(dZL-dZR)/(jP1-jP2)/DS;
            for(int j=0;j<Nj;j++){
                if(S[j][i][0][iPtn][0]>0.5){
                    double dZB=dZ2+mB*(j-jP2)*DS;
                    double dZT=dZ4+mT*(j-jP2)*DS;
                    mx=(dZT-dZB)/(iP3-iP2)/DS;
                    //corZ=dZR+my*(j-jP2)*DS;
                    corZ=dZB+mx*(i-iP2)*DS;
                    //printf("i=%d j=%d mx=%f my=%f corZ=%f\n",i,j,mx,my,corZ);
                    S[j][i][0][iPtn][1]=S[j][i][0][iPtn][1]-corZ;
                    S[j][i][1][iPtn][1]=tan(atan(S[j][i][1][iPtn][1])-atan(mx));
                    S[j][i][2][iPtn][1]=tan(atan(S[j][i][2][iPtn][1])-atan(my));
                    S[j][i][3][iPtn][1]=S[j][i][3][iPtn][1]-corZ;
                    S[j][i][4][iPtn][1]=tan(atan(S[j][i][4][iPtn][1])-atan(mx));
                    S[j][i][5][iPtn][1]=tan(atan(S[j][i][5][iPtn][1])-atan(my));
                }
            }
        }
    }

    //expo resampled data
    QString fn=dir+"resampledData.dat";
    QFile file2(fn);
    if (!file2.open(QIODevice::WriteOnly | QIODevice::Text)){
        printf("ERROR opening file=%s",fn.toStdString().c_str());
        return;
    }
    QTextStream out(&file2);
    QString sep;
    out<<"x(mm) y(mm) z(mm) tan(thetaX) tan(thetaY) devZ(mm) devTanX devzTanY"<<"\n";
    for(int i=0;i<Ni;i++){
        for(int j=0;j<Nj;j++){
            if(S[j][i][0][iPtn][0]>0.5){
                x=double(i-iP2)*DS;
                y=double(j-jP2)*DS;
                out <<x <<"\t"<< y << "\t";
                for(int iP=0;iP<Npar;iP++){
                    if(iP<Npar-1)
                        sep="\t";
                    else
                        sep="\n";
                    out<<S[j][i][iP][iPtn][1]<<sep;
                }
            }
        }
    }
    file2.close();
}


void RRcmp::checkRange(){
    printf("->checkRange with computing mean surface\n");
    Qt::CheckState state;
    //mean surface computing
    Nloaded=0;
    for(int iPtn=0;iPtn<5;iPtn++){
        state=idToCheckBox["checkBox_"+QString::number(iPtn)]->checkState();
        if(state==Qt::Checked)
            Nloaded++;
    }
    printf("\tmean-surface computed on %d participants\n",Nloaded);
    //S matrix initialization
    for(int i=0;i<Ni;i++){
        for(int j=0;j<Nj;j++){
            for(int iP=0;iP<Npar;iP++){
                S[j][i][iP][5][0]=0.;
                S[j][i][iP][5][1]=0.;
            }
        }
    }
    //mean
    for(int i=0;i<Ni;i++){
        for(int j=0;j<Nj;j++){
            for(int iPtn=0;iPtn<5;iPtn++){
                state=idToCheckBox["checkBox_"+QString::number(iPtn)]->checkState();
                if(state==Qt::Checked && S[j][i][0][iPtn][0]>0.5){
                    for(int iP=0;iP<Npar;iP++){
                        S[j][i][iP][5][0]++;
                        S[j][i][iP][5][1]=S[j][i][iP][5][1]+S[j][i][iP][iPtn][1];
                    }
                }
            }
        }
    }
    //Normalization
    for(int i=0;i<Ni;i++){
        for(int j=0;j<Nj;j++){
            for(int iP=0;iP<Npar;iP++){
                if(S[j][i][iP][5][0]>0.5){
                    S[j][i][iP][5][1]=S[j][i][iP][5][1]/S[j][i][iP][5][0];
                }
            }
        }
    }
    //reset S limits
    iMin=Ni;
    iMax=0;
    jMin=Nj;
    jMax=0;
    state=ui->checkBox_limComXY->checkState();
    int iComXY=0;
    if(state==Qt::Checked)
        iComXY=1;
    for(int iPtn=0;iPtn<5;iPtn++){
        state=idToCheckBox["checkBox_"+QString::number(iPtn)]->checkState();
        if(state==Qt::Checked){
            for(int iP=0;iP<Npar;iP++){
                range[iP][iPtn][0]=1.e+06; //Min
                range[iP][iPtn][1]=-1.e+06;//Max
                range[iP][iPtn][2]=0.;     //here used as SUMx
                range[iP][iPtn][3]=0.;     //here used as SUMxx
                range[iP][iPtn][4]=0.;     //here used as Ndata counter
            }
            //statistical analysis
            int Imin=Ni,Imax=0,Jmin=Nj,Jmax=0;
            for(int i=0;i<Ni;i++){
                for(int j=0;j<Nj;j++){
                    for(int iP=0;iP<Npar;iP++){
                        if((iComXY==0 && S[j][i][iP][iPtn][0]>0.5) || (iComXY==1 && S[j][i][iP][5][0]==Nloaded)){
                            iMin=min(iMin,i);
                            iMax=max(iMax,i);
                            jMin=min(jMin,j);
                            jMax=max(iMax,j);
                            Imin=min(Imin,i);
                            Imax=max(Imax,i);
                            Jmin=min(Jmin,j);
                            Jmax=max(Jmax,j);
                            range[iP][iPtn][0]=min(range[iP][iPtn][0],S[j][i][iP][iPtn][1]);
                            range[iP][iPtn][1]=max(range[iP][iPtn][1],S[j][i][iP][iPtn][1]);
                            range[iP][5][0]=min(range[iP][5][0],range[iP][iPtn][0]);
                            range[iP][5][1]=max(range[iP][5][1],range[iP][iPtn][1]);
                            range[iP][iPtn][2]=range[iP][iPtn][2]+S[j][i][iP][iPtn][1];
                            range[iP][iPtn][3]=range[iP][iPtn][3]+S[j][i][iP][iPtn][1]*S[j][i][iP][iPtn][1];
                            range[iP][iPtn][4]++;
                        }
                    }
                }
            }
            //printf("Imin=%d Imax=%d -> Xmin=%f Xmax=%f\n",Imin,Imax,double(Imin-iP2)*DS,double(Imax-iP2)*DS);
            //printf("Jmin=%d Jmax=%d -> Ymin=%f Ymax=%f\n",Jmin,Jmax,double(Jmin-jP2)*DS,double(Jmax-jP2)*DS);
            idToLineEdit["lineEdit_"+partnerChar[iPtn]+"xMin"]->setText(QString::number(double(Imin-iP2)*DS,'f',1));
            idToLineEdit["lineEdit_"+partnerChar[iPtn]+"xMax"]->setText(QString::number(double(Imax-iP2)*DS,'f',1));
            idToLineEdit["lineEdit_"+partnerChar[iPtn]+"yMin"]->setText(QString::number(double(Jmin-jP2)*DS,'f',1));
            idToLineEdit["lineEdit_"+partnerChar[iPtn]+"yMax"]->setText(QString::number(double(Jmax-jP2)*DS,'f',1));
            for(int iP=0;iP<Npar;iP++){
                //printf("range[%d][%d][0]=%f range[%d][%d][1]=%f\n",iP,iPtn,range[iP][iPtn][0],iP,iPtn,range[iP][iPtn][1]);
                range[iP][iPtn][2]=range[iP][iPtn][2]/range[iP][iPtn][4];      //mean
                range[iP][iPtn][3]=sqrt(range[iP][iPtn][3]/range[iP][iPtn][4]);//RMS
                range[iP][iPtn][4]=range[iP][iPtn][1]-range[iP][iPtn][0];      //Peak-Valley
            }
        }
    }

    displayAP();
}

void RRcmp::idealValue(){//values of the ideal parabolic profile
    printf("->idealValue\n");
    int iSpec=ui->comboBox_spec->currentIndex();
    double DELTAy=996.0;
    double DELTAx=982.0-1.4;//inner
    if(iSpec>2)
        DELTAx=871.0-1.4;//outer
    iP3=iP2+NINT(DELTAx/DS);
    jP1=jP2+NINT(DELTAy/DS);
    double focal=1710.;// focal lenght
    double xsiP2=372.25; //attaching point P2 (metallic-ball center)
    double etaP2=-0.76;
    double xsiP4=1325.52;//attaching point P4 (metallic-ball center)
    double etaP4=234.46;
    if(iSpec>2){
        xsiP2=1928.22; //attaching point P2 (metallic-ball center)
        etaP2=519.60;
        xsiP4=2652.77;//attaching point P4 (metallic-ball center)
        etaP4=1002.40;
    }
    double theta=atan((etaP4-etaP2)/(xsiP4-xsiP2));//rotation angle to overlap ParRF(xsi,eta) -> LabRF (x,z)
    double x,xsi1,eta1,A,B,C,xsi,eta,xP,zP,Dx;
    for(int iAP=0;iAP<2;iAP++){
        if(iAP==0)
            x=0.;//P2 abscissa in LabRF
        else
            x=DELTAx;//P4 abscissa in LabRF
        xsi1=xsiP2+x*cos(theta);
        eta1=etaP2+x*sin(theta);
        A=0.25/focal;
        B=1./tan(theta);
        C=-eta1-xsi1/tan(theta);
        xsi=(-B+sqrt(B*B-4.*A*C))/(2.*A);
        eta=0.25/focal*xsi*xsi;
        xP=(xsi-xsiP2)*cos(theta)+(eta-etaP2)*sin(theta);
        zP=-(xsi-xsiP2)*sin(theta)+(eta-etaP2)*cos(theta);
        Dx=tan(atan(0.5/focal*xsi)-theta);
        printf("xP=%f zP=%f Dx=%f\n",xP,zP,Dx);
        if(iAP==0){
            vP2[0]=zP;
            vP2[1]=Dx;
            vP2[2]=0.;
        }
        else{
            vP4[0]=zP;
            vP4[1]=Dx;
            vP4[2]=0.;
        }
    }
    printf("P1=(%f, %f)\n",(iP2-iP2)*DS,(jP1-jP2)*DS);
    printf("P2=(%f, %f)\n",(iP2-iP2)*DS,(jP2-jP2)*DS);
    printf("P3=(%f, %f)\n",(iP3-iP2)*DS,(jP1-jP2)*DS);
    printf("P4=(%f, %f)\n",(iP3-iP2)*DS,(jP2-jP2)*DS);
    //ideal parabola
    //A=0.25/focal;
    //B=1./tan(theta);
    for(int i=0;i<Ni;i++){
        x=(i-iP2)*DS;
        xsi1=xsiP2+x*cos(theta);
        eta1=etaP2+x*sin(theta);
        C=-eta1-xsi1/tan(theta);
        xsi=(-B+sqrt(B*B-4.*A*C))/(2.*A);
        eta=0.25/focal*xsi*xsi;
        //xP=(xsi-xsiP2)*cos(theta)+(eta-etaP2)*sin(theta);
        zP=-(xsi-xsiP2)*sin(theta)+(eta-etaP2)*cos(theta);
        Dx=tan(atan(0.5/focal*xsi)-theta);
        for(int j=0;j<Nj;j++){
            S[j][i][0][6][0]=1.;
            S[j][i][0][6][1]=zP;
            S[j][i][1][6][0]=1.;
            S[j][i][1][6][1]=Dx;
            for(int iPar=2;iPar<=5;iPar++){
                S[j][i][iPar][6][0]=1.;
                S[j][i][iPar][6][1]=0.;
            }
        }
    }
}


void RRcmp::displayAP(){
    int iPar=ui->comboBox_par->currentIndex();
    int iDec=ui->spinBox_decimal->value();
    printf("->displayAP for iPar=%d\n",iPar);
    QString Ptn0=ui->comboBox_iPtn0->currentText();
    QString Ptn1=ui->comboBox_iPtn1->currentText();
    printf("Ptn0=%s Ptn1=%s\n",Ptn0.toStdString().c_str(),Ptn1.toStdString().c_str());
    ui->comboBox_iPtn0->clear();
    ui->comboBox_iPtn1->clear();
    int iSWr=ui->comboBox_SWrealignment->currentIndex() ;
    double offsetP1=0.,offsetP2=0.,offsetP3=0.,offsetP4=0.;
    if(iPar<3){
        offsetP1=vP2[iPar];
        offsetP2=vP2[iPar];
        offsetP3=vP4[iPar];
        offsetP4=vP4[iPar];
    }
    ui->lineEdit_IP1->setText(QString::number(offsetP2,'f',iDec));
    ui->lineEdit_IP2->setText(QString::number(offsetP2,'f',iDec));
    ui->lineEdit_IP3->setText(QString::number(offsetP4,'f',iDec));
    ui->lineEdit_IP4->setText(QString::number(offsetP4,'f',iDec));
    Qt::CheckState state;
    state=ui->checkBox_mrad->checkState();
    double fact=1.;
    if(state==Qt::Checked)
        fact=1000.;
    state=ui->checkBox_showDiff->checkState();
    int iShowDiff=0;
    if(state==Qt::Checked)
        iShowDiff=1;
    int iMaxIdx=-1;
    double sx=0.,sxx=0.;
    for(int iPtn=0;iPtn<Nptn;iPtn++){
        state=idToCheckBox["checkBox_"+QString::number(iPtn)]->checkState();
        if(state==Qt::Checked){
            if(iSWr==2 && iPar==3){
                offsetP1=S[jP1][iP2][3][iPtn][1];
                offsetP2=S[jP2][iP2][3][iPtn][1];
                offsetP3=S[jP1][iP3][3][iPtn][1];
                offsetP4=S[jP2][iP3][3][iPtn][1];
            }
            //printf("offsetP1=%f offsetP2=%f offsetP3=%f offsetP4=%f\n",offsetP1,offsetP3,offsetP3,offsetP4);
            if(iShowDiff==0){
                offsetP1=0.0;
                offsetP2=0.0;
                offsetP3=0.0;
                offsetP4=0.0;
            }
            printf("%f\t%f\n%f\t%f\n",S[jP1][iP2][iPar][iPtn][1]-offsetP1,S[jP1][iP3][iPar][iPtn][1]-offsetP3,
                                      S[jP2][iP2][iPar][iPtn][1]-offsetP2,S[jP2][iP3][iPar][iPtn][1]-offsetP4);
            idToLineEdit["lineEdit_"+partnerChar[iPtn]+"P1"]->setText(QString::number((S[jP1][iP2][iPar][iPtn][1]-offsetP1)*fact,'f',iDec));
            idToLineEdit["lineEdit_"+partnerChar[iPtn]+"P2"]->setText(QString::number((S[jP2][iP2][iPar][iPtn][1]-offsetP2)*fact,'f',iDec));
            idToLineEdit["lineEdit_"+partnerChar[iPtn]+"P3"]->setText(QString::number((S[jP1][iP3][iPar][iPtn][1]-offsetP3)*fact,'f',iDec));
            idToLineEdit["lineEdit_"+partnerChar[iPtn]+"P4"]->setText(QString::number((S[jP2][iP3][iPar][iPtn][1]-offsetP4)*fact,'f',iDec));
            idToLineEdit["lineEdit_"+partnerChar[iPtn]+"min"]->setText(QString::number((range[iPar][iPtn][0])*fact,'f',iDec));
            idToLineEdit["lineEdit_"+partnerChar[iPtn]+"max"]->setText(QString::number((range[iPar][iPtn][1])*fact,'f',iDec));
            //statistics
            idToLineEdit["lineEdit_"+partnerChar[iPtn]+"_Mean"]->setText(QString::number((range[iPar][iPtn][2])*fact,'f',iDec));
            idToLineEdit["lineEdit_"+partnerChar[iPtn]+"_RMS"]->setText(QString::number((range[iPar][iPtn][3])*fact,'f',iDec));
            idToLineEdit["lineEdit_"+partnerChar[iPtn]+"_PV"]->setText(QString::number((range[iPar][iPtn][4])*fact,'f',iDec));
            idToLineEdit["lineEdit_"+partnerChar[iPtn]+"_Min"]->setText(QString::number((range[iPar][iPtn][0])*fact,'f',iDec));
            idToLineEdit["lineEdit_"+partnerChar[iPtn]+"_Max"]->setText(QString::number((range[iPar][iPtn][1])*fact,'f',iDec));
            printf("stat: %f %f %f %f %f\n",
                   range[iPar][iPtn][2],range[iPar][iPtn][3],range[iPar][iPtn][4],range[iPar][iPtn][0],range[iPar][iPtn][1]);
            ui->comboBox_iPtn0->addItem(partner[iPtn]);
            ui->comboBox_iPtn1->addItem(partner[iPtn]);
            sx=sx+range[iPar][iPtn][3];
            sxx=sxx+range[iPar][iPtn][3]*range[iPar][iPtn][3];
            iMaxIdx++;
        }
        else{
            idToLineEdit["lineEdit_"+partnerChar[iPtn]+"P1"] ->setText("");
            idToLineEdit["lineEdit_"+partnerChar[iPtn]+"P2"] ->setText("");
            idToLineEdit["lineEdit_"+partnerChar[iPtn]+"P3"] ->setText("");
            idToLineEdit["lineEdit_"+partnerChar[iPtn]+"P4"] ->setText("");
            idToLineEdit["lineEdit_"+partnerChar[iPtn]+"min"]->setText("");
            idToLineEdit["lineEdit_"+partnerChar[iPtn]+"max"]->setText("");
            //statistics
            idToLineEdit["lineEdit_"+partnerChar[iPtn]+"_Mean"]->setText("");
            idToLineEdit["lineEdit_"+partnerChar[iPtn]+"_RMS"] ->setText("");
            idToLineEdit["lineEdit_"+partnerChar[iPtn]+"_PV"]  ->setText("");
            idToLineEdit["lineEdit_"+partnerChar[iPtn]+"_Min"] ->setText("");
            idToLineEdit["lineEdit_"+partnerChar[iPtn]+"_Max"] ->setText("");
        }
    }
    //restore partners to be compared
    if(iMaxIdx>=0){
        ui->comboBox_iPtn0->addItem(partner[5]);//Mean option
        ui->comboBox_iPtn1->addItem(partner[5]);
        iMaxIdx++;
        ui->comboBox_iPtn0->addItem(partner[6]);//Ideal parabola option
        ui->comboBox_iPtn1->addItem(partner[6]);
        iMaxIdx++;
        int iCix=-1;
        QString Ptn0new,Ptn1new;
        do{
            iCix++;
            ui->comboBox_iPtn0->setCurrentIndex(iCix);
            Ptn0new=ui->comboBox_iPtn0->currentText();
        }while(Ptn0new!=Ptn0 && iCix<iMaxIdx);
        iCix=-1;
        do{
            iCix++;
            ui->comboBox_iPtn1->setCurrentIndex(iCix);
            Ptn1new=ui->comboBox_iPtn1->currentText();
        }while(Ptn1new!=Ptn1 && iCix<iMaxIdx);
    }
    ui->lineEdit_min->setText(QString::number(range[iPar][5][0]));
    ui->lineEdit_max->setText(QString::number(range[iPar][5][1]));
    sx=sx/double(Nloaded);
    sxx=sxx/double(Nloaded);
    ui->lineEdit_meanRMS->setText(QString::number(sx*fact,'f',iDec)+"+-"+QString::number(sqrt(sxx-sx*sx)*fact,'f',iDec));//RMS mean and std
}

void RRcmp::map0(){
    plotMap(0);
}

void RRcmp::map1(){
    plotMap(1);
}
void RRcmp::map2(){
    plotMap(2);
}
void RRcmp::map3(){
    plotMap(3);
}
void RRcmp::map4(){
    plotMap(4);
}

void RRcmp::plotMap(int iPtn){
    int iSpec=ui->comboBox_spec->currentIndex();
    int iPar=ui->comboBox_par->currentIndex();
    printf("->plotMat iPtn=%d iSpec=%d iPar=%d\n",iPtn,iSpec,iPar);
    Mat map(iMax-iMin+1,jMax-jMin+1,CV_8UC3,Scalar(150,150,150));
    range[iPar][5][0]=ui->lineEdit_min->text().toDouble();
    range[iPar][5][1]=ui->lineEdit_max->text().toDouble();
    double rng=range[iPar][5][1]-range[iPar][5][0];
    double val;
    for(int i=iMin;i<=iMax;i++){
        for(int j=jMin;j<=jMax;j++){
            if(S[j][i][iPar][iPtn][0]>0.5){
                val=(S[j][i][iPar][iPtn][1]-range[iPar][5][0])/rng;
                pxColor(val);
                for(int k=0;k<3;k++)
                    map.at<Vec3b>(iMax-i,jMax-j)[k] = static_cast<uint8_t>(Vservice[k]);
            }
        }
    }
    QString nameW=partner[iPtn]+" "+specim[iSpec]+" "+param[iPar];
    string namWin=nameW.toStdString();
    namedWindow(namWin, WINDOW_NORMAL);
    moveWindow(namWin,90+nOpenGraph*5,90+nOpenGraph*5);
    imshow(namWin,map);
    map.release();
    nOpenGraph++;

    //plot profile-graph
    int nDatR=0,nDatL=0;
    int j;
    double xPlotR[Ni],xPlotL[Ni],yPlotR[Ni],yPlotL[Ni];
    for(int i=iMin;i<=iMax;i++){
        j=jP2;
        if(S[j][i][iPar][iPtn][0]>0.5){
            xPlotR[nDatR]=DS*(i-iP2);
            yPlotR[nDatR]=S[j][i][iPar][iPtn][1];
            //printf("nDat=%d xPlot=%f yPlot=%f\n",nDat,xPlot[nDat],yPlot[nDat]);
            nDatR++;
        }
        j=jP1;
        if(S[j][i][iPar][iPtn][0]>0.5){
            xPlotL[nDatL]=DS*(i-iP2);
            yPlotL[nDatL]=S[j][i][iPar][iPtn][1];
            //printf("nDat=%d xPlot=%f yPlot=%f\n",nDat,xPlot[nDat],yPlot[nDat]);
            nDatL++;
        }
    }

    G1=new QwtPlot();
    G1->setGeometry(90+nOpenGraph*5,90+nOpenGraph*5,500,300);
    QwtPlotCurve *curve1=new QwtPlotCurve("curve1");
    curve1->setSamples(xPlotR,yPlotR,nDatR);
    curve1->setPen(QPen(Qt::green,2,Qt::SolidLine));
    curve1->attach(G1);
    QwtPlotCurve *curve2=new QwtPlotCurve("curve2");
    curve2->setSamples(xPlotL,yPlotL,nDatL);
    curve2->setPen(QPen(Qt::red,2,Qt::SolidLine));
    curve2->attach(G1);
    //draw vertical lines at the attaching points
    double xP[2],yP[2];
    xP[0]=0.;
    xP[1]=0.;
    yP[0]=range[iPar][5][0];
    yP[1]=range[iPar][5][1];
    QwtPlotCurve *curve3=new QwtPlotCurve("curve3");
    curve3->setSamples(xP,yP,2);
    curve3->setPen(QPen(Qt::black,1,Qt::DashLine));
    curve3->attach(G1);
    xP[0]=(iP3-iP2)*DS;
    xP[1]=(iP3-iP2)*DS;
    QwtPlotCurve *curve4=new QwtPlotCurve("curve4");
    curve4->setSamples(xP,yP,2);
    curve4->setPen(QPen(Qt::black,1,Qt::DashLine));
    curve4->attach(G1);
    //draw y=0
    if(yP[0]<0. && yP[1]>0.){
        xP[0]=double(iMin-iP2)*DS;
        xP[1]=double(iMax-iP2)*DS;
        yP[0]=0.;
        yP[1]=0.;
        QwtPlotCurve *curve5=new QwtPlotCurve("curve5");
        curve5->setSamples(xP,yP,2);
        curve5->setPen(QPen(Qt::black,1,Qt::DashLine));
        curve5->attach(G1);
    }

    G1-> setAxisTitle(0,param[iPar]);
    G1-> setAxisTitle(2,"x (mm)");
    G1-> setAxisScale(0,range[iPar][5][0],range[iPar][5][1],0);
    G1-> setTitle(nameW);
    G1-> setAutoReplot();
    G1-> show();
    nOpenGraph++;
    if(nOpenGraph>50)
        nOpenGraph=0;
}

void RRcmp::compare(){
    int iPar=ui->comboBox_par->currentIndex();
    int iDec=ui->spinBox_decimal->value();
    QString Ptn0=ui->comboBox_iPtn0->currentText();
    QString Ptn1=ui->comboBox_iPtn1->currentText();
    int iPtn0=0,iPtn1=0;
    if(!Ptn0.isEmpty()){
        while(Ptn0!=partner[iPtn0]){
            iPtn0++;
        }
    }
    if(!Ptn1.isEmpty()){
        while(Ptn1!=partner[iPtn1]){
            iPtn1++;
        }
    }
    if(iPtn0==iPtn1)
        return;
    printf("->compare iPtn0=%d iPtn1=%d\n",iPtn0,iPtn1);
    Qt::CheckState state;
    state=ui->checkBox_limComXY->checkState();
    int iComXY=0;
    if(state==Qt::Checked)
        iComXY=1;
    if(iPtn0!=5 && iPtn1!=5)
        iComXY=0;
    int ndat=0;
    double Sx=0.,Sxx=0.,delta,PV,deltaMin=1.e+06,deltaMax=-1.e+06;
    for(int i=0;i<Ni;i++){
        for(int j=0;j<Nj;j++){
            if((iComXY==0 && S[j][i][iPar][iPtn0][0]>0.5 && S[j][i][iPar][iPtn1][0]>0.5) ||
                (iComXY==1 && S[j][i][iPar][5][0]==Nloaded)){
                delta=S[j][i][iPar][iPtn1][1]-S[j][i][iPar][iPtn0][1];
                deltaMin=min(deltaMin,delta);
                deltaMax=max(deltaMax,delta);
                Sx=Sx+delta;
                Sxx=Sxx+delta*delta;
                ndat++;
            }
        }
    }
    PV=deltaMax-deltaMin;
    double Mean=Sx/ndat;
    double RMS=sqrt(Sxx/ndat);
    Qt::CheckState state0;
    state0=ui->checkBox_mrad->checkState();
    double fact=1.;
    if(state0==Qt::Checked)
        fact=1000.;
    ui->lineEdit_mean->setText(QString::number(Mean*fact,'f',iDec));
    ui->lineEdit_RMS->setText(QString::number(RMS*fact,'f',iDec));
    ui->lineEdit_PV->setText(QString::number(PV*fact,'f',iDec));
    ui->lineEdit_diffMin->setText(QString::number(deltaMin*fact,'f',iDec));
    ui->lineEdit_diffMax->setText(QString::number(deltaMax*fact,'f',iDec));

    //write the results in logfile
    int iSpec=ui->comboBox_spec->currentIndex();
    QString nameW="Diff_"+partner[iPtn1]+"_"+partner[iPtn0]+"_"+specim[iSpec]+"_"+param[iPar];
    QFile fLog(dir+"/fLog.txt");
    if (!fLog.open(QIODevice::WriteOnly | QIODevice::Append))
        return;
    QTextStream out(&fLog);
    //out<<Mean<<"\t"<<standDev<<"\t"<<PV<<"\t"<<deltaMin<<"\t"<<deltaMax<<"\n";
    out<<nameW.toStdString().c_str()<<"\t"<<Mean<<"\t"<<RMS<<"\t"<<PV<<"\t"<<deltaMin<<"\t"<<deltaMax<<"\n";
    fLog.close();

    state=ui->checkBox_mapDiff->checkState();
    if(state==Qt::Checked){
        double MapMin,MapMax;
        QString string;
        string= ui->lineEdit_2DmapMin->text();
        if(string==""){
            MapMin=deltaMin;
            ui->lineEdit_2DmapMin->setText(QString::number(deltaMin*fact,'f',iDec));
        }
        else
            MapMin=string.toDouble();
        string= ui->lineEdit_2DmapMax->text();
        if(string==""){
            MapMax=deltaMax;
            ui->lineEdit_2DmapMax->setText(QString::number(deltaMax*fact,'f',iDec));
        }
        else
            MapMax=string.toDouble();
        plotMapDiff(iPtn0,iPtn1,MapMin,MapMax);
    }
}

void RRcmp::plotMapDiff(int iPtn0, int iPtn1, double deltaMin, double deltaMax){
    int iSpec=ui->comboBox_spec->currentIndex();
    int iPar=ui->comboBox_par->currentIndex();
    printf("->plotMatDiff iPtn0==%d iPtn1=%d iSpec=%d iPar=%d\n",iPtn0,iPtn1,iSpec,iPar);
    Qt::CheckState state;
    state=ui->checkBox_limComXY->checkState();
    int iComXY=0;
    if(state==Qt::Checked)
        iComXY=1;
    if(iPtn0!=5 && iPtn1!=5)
        iComXY=0;
    Mat map(iMax-iMin+1,jMax-jMin+1,CV_8UC3,Scalar(150,150,150));
    double rng=deltaMax-deltaMin;
    double val;
    for(int i=iMin;i<=iMax;i++){
        for(int j=jMin;j<=jMax;j++){
            if((iComXY==0 && S[j][i][iPar][iPtn0][0]>0.5 && S[j][i][iPar][iPtn1][0]>0.5) ||
                (iComXY==1 && S[j][i][iPar][5][0]==Nloaded)){
                val=((S[j][i][iPar][iPtn1][1]-S[j][i][iPar][iPtn0][1])-deltaMin)/rng;
                pxColor(val);
                for(int k=0;k<3;k++)
                    map.at<Vec3b>(iMax-i,jMax-j)[k] = static_cast<uint8_t>(Vservice[k]);
            }
        }
    }
    QString nameW="Diff "+partner[iPtn1]+"-"+partner[iPtn0]+" "+specim[iSpec]+" "+param[iPar];
    string namWin=nameW.toStdString();
    namedWindow(namWin, WINDOW_NORMAL);
    moveWindow(namWin,90+nOpenGraph*5,90+nOpenGraph*5);
    imshow(namWin,map);
    map.release();
    nOpenGraph++;

    //plot profile-graph
    int nDatR=0,nDatL=0;
    int j;
    double xPlotR[Ni],xPlotL[Ni],yPlotR[Ni],yPlotL[Ni];
    for(int i=iMin;i<=iMax;i++){
        j=jP2;
        if(S[j][i][iPar][iPtn0][0]>0.5 && S[j][i][iPar][iPtn1][0]>0.5){
            xPlotR[nDatR]=DS*(i-iP2);
            yPlotR[nDatR]=S[j][i][iPar][iPtn1][1]-S[j][i][iPar][iPtn0][1];
            //printf("nDat=%d xPlot=%f yPlot=%f\n",nDat,xPlot[nDat],yPlot[nDat]);
            nDatR++;
        }
        j=jP1;
        if(S[j][i][iPar][iPtn0][0]>0.5 && S[j][i][iPar][iPtn1][0]>0.5){
            xPlotL[nDatL]=DS*(i-iP2);
            yPlotL[nDatL]=S[j][i][iPar][iPtn1][1]-S[j][i][iPar][iPtn0][1];
            //printf("nDat=%d xPlot=%f yPlot=%f\n",nDat,xPlot[nDat],yPlot[nDat]);
            nDatL++;
        }
    }
    G1=new QwtPlot();
    G1->setGeometry(90+nOpenGraph*5,90+nOpenGraph*5,500,300);
    QwtPlotCurve *curve1=new QwtPlotCurve("curve1");
    curve1->setSamples(xPlotR,yPlotR,nDatR);
    curve1->setPen(QPen(Qt::green,2,Qt::SolidLine));
    curve1->attach(G1);
    QwtPlotCurve *curve2=new QwtPlotCurve("curve2");
    curve2->setSamples(xPlotL,yPlotL,nDatL);
    curve2->setPen(QPen(Qt::red,2,Qt::SolidLine));
    curve2->attach(G1);
    //draw vertical lines at the attaching points
    double xP[2],yP[2];
    xP[0]=0.;
    xP[1]=0.;
    yP[0]=range[iPar][5][0];
    yP[1]=range[iPar][5][1];
    QwtPlotCurve *curve3=new QwtPlotCurve("curve3");
    curve3->setSamples(xP,yP,2);
    curve3->setPen(QPen(Qt::black,1,Qt::DashLine));
    curve3->attach(G1);
    xP[0]=(iP3-iP2)*DS;
    xP[1]=(iP3-iP2)*DS;
    QwtPlotCurve *curve4=new QwtPlotCurve("curve4");
    curve4->setSamples(xP,yP,2);
    curve4->setPen(QPen(Qt::black,1,Qt::DashLine));
    curve4->attach(G1);
    //draw y=0
    if(yP[0]<0. && yP[1]>0.){
        xP[0]=double(iMin-iP2)*DS;
        xP[1]=double(iMax-iP2)*DS;
        yP[0]=0.;
        yP[1]=0.;
        QwtPlotCurve *curve5=new QwtPlotCurve("curve5");
        curve5->setSamples(xP,yP,2);
        curve5->setPen(QPen(Qt::black,1,Qt::DashLine));
        curve5->attach(G1);
    }
    G1-> setAxisTitle(0,"Diff "+param[iPar]);
    G1-> setAxisTitle(2,"x (mm)");
    G1-> setAxisScale(0,deltaMin,deltaMax,0);
    G1-> setTitle(nameW);
    G1-> setAutoReplot();
    G1-> show();
    nOpenGraph++;
    if(nOpenGraph>50)
        nOpenGraph=0;
}


int NINT(double x){
    int n;
    if(x>=0.)
        n=int(x+0.5);
    else
        n=int(x-0.5);
    return(n);
}

void pxColor(double val){
    //compute the pixel color to build a color map
    int Blue=0;
    int Green=0;
    int Red=0;
    if(val==0.){
        Blue=255;
        Green=0;
        Red=0;
    }
    else if(val>0. && val< 0.25){
        Blue=255;
        Green=int(255.*4.*val+0.5);
        Red=0;
    }
    else if(val>= 0.25 && val< 0.50){
        Blue=int(255.*(1.-4.*(val-0.25)));
        Green=255;
        Red=0;
    }
    else if(val>= 0.50 && val< 0.75){
        Blue=0;
        Green=255;
        Red=int(255.*4.*(val-0.50));
    }
    else if(val>= 0.75 && val<=1.0){
        Blue=0;
        Green=int(255.*(1.-4.*(val-0.75)));
        Red=255;
    }
    else if(val>1){
        Blue=255;
        Green=255;
        Red=255;
    }

    Vservice[0]=Blue;
    Vservice[1]=Green;
    Vservice[2]=Red;
}
