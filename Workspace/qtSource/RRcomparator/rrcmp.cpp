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

using namespace std;
using namespace cv;

//sampling matrix
const int Nj=181;
const int Ni=171;
const int Nptn=5;//N. partners
const int Npar=6;//N. parameters
//sampling matrix
double S[Nj][Ni][Npar][Nptn][2];
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
//      [j][i][l][k][0] = N. of input data
//      [j][i][l][k][1] = mean value
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
double range[Npar][Nptn+1][2];
//          [iPar][iPtn][0] min
//          [iPar][iPtn][1] max
double Vservice[3];

QString dir="/run/media/marco/d2quadra/VISproPT/RoundRobin/";
QString partner[Nptn]={"ENEA","FISE","DLR","NREL","SANDIA"};
QString partnerChar[Nptn]={"E","F","D","N","S"};
QString param[Npar]={"z","Dx","Dy","devZ","devDx","devDy"};
QString specim[6]={"inner#60","inner#61","inner#62","outer#93","outer#97","outer#99"};

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
    printf("         version 1.0 10 November 2023\n\n");
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
    connect(ui->comboBox_spec, SIGNAL(currentIndexChanged(int)),this,SLOT(loadAll()));
    connect(ui->comboBox_par, SIGNAL(currentIndexChanged(int)),this,SLOT(displayAP()));
    connect(ui->pushButton_compare, SIGNAL(pressed()),this,SLOT(compare()));
    connect(ui->pushButton_map_0, SIGNAL(pressed()),this,SLOT(map0()));
    connect(ui->pushButton_map_1, SIGNAL(pressed()),this,SLOT(map1()));
    connect(ui->pushButton_map_2, SIGNAL(pressed()),this,SLOT(map2()));
    connect(ui->pushButton_map_3, SIGNAL(pressed()),this,SLOT(map3()));
    connect(ui->pushButton_map_4, SIGNAL(pressed()),this,SLOT(map4()));

//    idToLineEdit["lineEdit_IP1"]=ui->lineEdit_IP1;
//    idToLineEdit["lineEdit_IP2"]=ui->lineEdit_IP2;
//    idToLineEdit["lineEdit_IP3"]=ui->lineEdit_IP3;
//    idToLineEdit["lineEdit_IP4"]=ui->lineEdit_IP4;
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
    //reset S limits
    iMin=Ni;
    iMax=0;
    jMin=Nj;
    jMax=0;
    //reset common range
    for(int iP=0;iP<Npar;iP++){
        range[iP][5][0]=1.e+06;
        range[iP][5][1]=-1.e+06;
    }
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
}

void RRcmp::loadENEA(){
    Qt::CheckState state=ui->checkBox_0 -> checkState();
    if( state != Qt::Checked )
        return;
    loadData(0);
}

void RRcmp::loadFISE(){
    Qt::CheckState state=ui->checkBox_1 -> checkState();
    if( state != Qt::Checked )
        return;
    loadData(1);
}

void RRcmp::loadDLR(){
    Qt::CheckState state=ui->checkBox_2 -> checkState();
    if( state != Qt::Checked )
        return;
    loadData(2);
}

void RRcmp::loadNREL(){
    Qt::CheckState state=ui->checkBox_3 -> checkState();
    if( state != Qt::Checked )
        return;
    loadData(3);
}

void RRcmp::loadSANDIA(){
    Qt::CheckState state=ui->checkBox_4 -> checkState();
    if( state != Qt::Checked )
        return;
    loadData(4);
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
        List =line.split(" ");
        int nV=List.count();
        iC++;
        if(nV!=8){
            printf("ERROR: data file must consist of 8 column; @iC=%d found %d\n",iC,nV);
            printf("line0 %s\n",line.toStdString().c_str());
        }
        pezzo=List.at(0).toLocal8Bit().constData();
        x=pezzo.toDouble();
        pezzo=List.at(1).toLocal8Bit().constData();
        y=pezzo.toDouble();
        int i=NINT(x/DS)+iP2;
        int j=NINT(y/DS)+jP2;
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
    //S matrix normalization to Ndata per cell
    int Imin=Ni,Imax=0,Jmin=Nj,Jmax=0;
    for(int i=0;i<Ni;i++){
        for(int j=0;j<Nj;j++){
            for(int iP=0;iP<Npar;iP++){
                if(S[j][i][iP][iPtn][0]>0.5){
                    S[j][i][iP][iPtn][1]=S[j][i][iP][iPtn][1]/S[j][i][iP][iPtn][0];
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
                }
            }
        }
    }
    printf("Imin=%d Imax=%d -> Xmin=%f Xmax=%f\n",Imin,Imax,double(Imin-iP2)*DS,double(Imax-iP2)*DS);
    printf("Jmin=%d Jmax=%d -> Ymin=%f Ymax=%f\n",Jmin,Jmax,double(Jmin-jP2)*DS,double(Jmax-jP2)*DS);
    idToLineEdit["lineEdit_"+partnerChar[iPtn]+"xMin"]->setText(QString::number(double(Imin-iP2)*DS,'f',1));
    idToLineEdit["lineEdit_"+partnerChar[iPtn]+"xMax"]->setText(QString::number(double(Imax-iP2)*DS,'f',1));
    idToLineEdit["lineEdit_"+partnerChar[iPtn]+"yMin"]->setText(QString::number(double(Jmin-jP2)*DS,'f',1));
    idToLineEdit["lineEdit_"+partnerChar[iPtn]+"yMax"]->setText(QString::number(double(Jmax-jP2)*DS,'f',1));
    for(int iP=0;iP<Npar;iP++)
        printf("range[%d][%d][0]=%f range[%d][%d][1]=%f\n",iP,iPtn,range[iP][iPtn][0],iP,iPtn,range[iP][iPtn][1]);
    int iPar=ui->comboBox_par->currentIndex();
    idToLineEdit["lineEdit_"+partnerChar[iPtn]+"min"]->setText(QString::number(range[iPar][iPtn][0],'e',3));
    idToLineEdit["lineEdit_"+partnerChar[iPtn]+"max"]->setText(QString::number(range[iPar][iPtn][1],'e',3));
    ui->lineEdit_min->setText(QString::number(range[iPar][5][0],'e',3));
    ui->lineEdit_max->setText(QString::number(range[iPar][5][1],'e',3));
    jP1=jP2+NINT(996./DS);
    double DELTAx=982-1.4;//inner
    if(iSpec>2)
        DELTAx=871-1.4;//outer
    iP3=iP2+NINT(DELTAx/DS);
    displayAP();
    printf("P1=(%f, %f)\n",(iP2-iP2)*DS,(jP1-jP2)*DS);
    printf("P2=(%f, %f)\n",(iP2-iP2)*DS,(jP2-jP2)*DS);
    printf("P3=(%f, %f)\n",(iP3-iP2)*DS,(jP1-jP2)*DS);
    printf("P4=(%f, %f)\n",(iP3-iP2)*DS,(jP2-jP2)*DS);
    //ideal values
    idealValue();

    //expo resampled data
    QString fn=dir+"resampledData.dat";
    QFile file2(fn);
    if (!file2.open(QIODevice::WriteOnly | QIODevice::Text)){
        printf("ERROR opening file=%s",fn.toStdString().c_str());
        return;
    }
    QTextStream out(&file2);
    QString sep;
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

void RRcmp::idealValue(){//values of the ideal parabolic profile
    int iSpec=ui->comboBox_spec->currentIndex();
    int iPar=ui->comboBox_par->currentIndex();
    double DELTAx=982-1.4;//inner
    if(iSpec>2)
        DELTAx=871-1.4;//outer
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
            x=0;//P2 abscissa in LabRF
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
        double val2print;
        if(iPar==0)
            val2print=zP;
        else if(iPar==1)
            val2print=Dx;
        else
            val2print=0.;
        if(iAP==0){
            ui->lineEdit_IP1->setText(QString::number(val2print,'e',3));
            ui->lineEdit_IP2->setText(QString::number(val2print,'e',3));
        }
        else{
            ui->lineEdit_IP3->setText(QString::number(val2print,'e',3));
            ui->lineEdit_IP4->setText(QString::number(val2print,'e',3));
        }
    }
}


void RRcmp::displayAP(){
    int iPar=ui->comboBox_par->currentIndex();
    printf("->displayAP for iPar=%d\n",iPar);
    ui->comboBox_iPtn0->clear();
    ui->comboBox_iPtn1->clear();
    Qt::CheckState state;
    for(int iPtn=0;iPtn<Nptn;iPtn++){
        state=idToCheckBox["checkBox_"+QString::number(iPtn)]->checkState();
        if(state==Qt::Checked){
            idToLineEdit["lineEdit_"+partnerChar[iPtn]+"P2"]->setText(QString::number(S[jP2][iP2][iPar][iPtn][1],'e',3));
            idToLineEdit["lineEdit_"+partnerChar[iPtn]+"P1"]->setText(QString::number(S[jP1][iP2][iPar][iPtn][1],'e',3));
            idToLineEdit["lineEdit_"+partnerChar[iPtn]+"P3"]->setText(QString::number(S[jP1][iP3][iPar][iPtn][1],'e',3));
            idToLineEdit["lineEdit_"+partnerChar[iPtn]+"P4"]->setText(QString::number(S[jP2][iP3][iPar][iPtn][1],'e',3));
            idToLineEdit["lineEdit_"+partnerChar[iPtn]+"min"]->setText(QString::number(range[iPar][iPtn][0],'e',3));
            idToLineEdit["lineEdit_"+partnerChar[iPtn]+"max"]->setText(QString::number(range[iPar][iPtn][1],'e',3));
            ui->comboBox_iPtn0->addItem(partner[iPtn]);
            ui->comboBox_iPtn1->addItem(partner[iPtn]);
        }
    }
    ui->lineEdit_min->setText(QString::number(range[iPar][5][0]));
    ui->lineEdit_max->setText(QString::number(range[iPar][5][1]));
    idealValue();
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
    imshow(namWin,map);
}

void RRcmp::compare(){
    int iPar=ui->comboBox_par->currentIndex();
    int iPtn0=ui->comboBox_iPtn0->currentIndex();
    int iPtn1=ui->comboBox_iPtn1->currentIndex();
    if(iPtn0==iPtn1)
        return;
    int ndat=0;
    double Sx=0.,Sxx=0.,delta,PV,deltaMin=1.e+06,deltaMax=-1.e+06;
    for(int i=0;i<Ni;i++){
        for(int j=0;j<Nj;j++){
            if(S[j][i][iPar][iPtn0][0]>0.5 && S[j][i][iPar][iPtn1][0]>0.5){
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
    double standDev=Sxx/ndat-Mean*Mean;
    ui->lineEdit_mean->setText(QString::number(Mean,'e',2));
    if(standDev>0.){
        standDev=sqrt(standDev);
        ui->lineEdit_RMS->setText(QString::number(standDev,'e',2));
    }
    else
        ui->lineEdit_RMS->setText("NAN");
    ui->lineEdit_PV->setText(QString::number(PV,'e',1));
    ui->lineEdit_diffMin->setText(QString::number(deltaMin,'e',1));
    ui->lineEdit_diffMax->setText(QString::number(deltaMax,'e',1));
    Qt::CheckState state;
    state=ui->checkBox_mapDiff->checkState();
    if(state==Qt::Checked)
        plotMapDiff(iPtn0,iPtn1,deltaMin,deltaMax);
}

void RRcmp::plotMapDiff(int iPtn0, int iPtn1, double deltaMin, double deltaMax){
    int iSpec=ui->comboBox_spec->currentIndex();
    int iPar=ui->comboBox_par->currentIndex();
    printf("->plotMatDiff iPtn==%d iPtn1=%d iSpec=%d iPar=%d\n",iPtn0,iPtn1,iSpec,iPar);
    Mat map(iMax-iMin+1,jMax-jMin+1,CV_8UC3,Scalar(150,150,150));
    double rng=deltaMax-deltaMin;
    double val;
    for(int i=iMin;i<=iMax;i++){
        for(int j=jMin;j<=jMax;j++){
            if(S[j][i][iPar][iPtn0][0]>0.5 && S[j][i][iPar][iPtn1][0]>0.5){
                val=((S[j][i][iPar][iPtn1][1]-S[j][i][iPar][iPtn0][1])-deltaMin)/rng;
                pxColor(val);
                for(int k=0;k<3;k++)
                    map.at<Vec3b>(iMax-i,jMax-j)[k] = static_cast<uint8_t>(Vservice[k]);
            }
        }
    }
    QString nameW="Diff "+partner[iPtn0]+"&"+partner[iPtn1]+" "+specim[iSpec]+" "+param[iPar];
    string namWin=nameW.toStdString();
    namedWindow(namWin, WINDOW_NORMAL);
    imshow(namWin,map);
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
