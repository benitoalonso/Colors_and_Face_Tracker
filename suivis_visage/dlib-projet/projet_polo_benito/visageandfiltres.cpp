#include<dlib/image_processing/frontal_face_detector.h>
#include<dlib/image_processing.h>
#include<dlib/opencv.h>

#include<opencv2/imgproc.hpp>
#include<opencv2/highgui.hpp>
#include<opencv2/photo.hpp>
#include <opencv2/core.hpp>

#include <stdlib.h>
#include <gtk/gtk.h>

#include <iostream>
#include <string>
#include <math.h>
#include <stdio.h>
#include <time.h>

#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif

using namespace cv;
using namespace std;
using namespace dlib;

//--------------------definition des tailles et para position
#define WIDcam  480 //resolution de la webcam
#define LENcam  640

#define WIDlunettes  2498    //taille de l'image lunettes
#define LENlunettes  5885

#define WIDnez 800  //taille de l'image nez clown

#define WIDmasque 623    //taille de l'image masque
#define LENmasque 1065

//--------------------gestion des flags pour les différents filtres et boutons
#define NUMflags    9   //en tout nous avons 7 filtres, un bouton screenshot et un bouton quitter
#define NUMdata     7   //taille mémoire données pout les filtres

int flagtextlabel=0; //gestion speciale pour le label
const gchar *sText;

int flagsuivisvisage=0;
#define DELTA 50  //deadzone centrale
int posX=320; //pour strocker centre du visage (référence sur le point n°30 du nez)
int posY=240;

int nb_visages=0; //nombre de visages détectés

int maximumm(int a, int b){
    if (a>b) return a;
    return b;
}

int minimumm(int a, int b){
    if (a>b) return b;
    return a;
}

//--------------------Déclaration des fonctions avec gtk
void on_activate_entry(GtkWidget *pEntry, gpointer data);
void on_copier_button_miroirdroit(GtkWidget *pButton, gpointer data);
void on_copier_button_miroirgauche(GtkWidget *pButton, gpointer data);
void on_copier_button_dessin(GtkWidget *pButton, gpointer data);
void on_copier_button_lunettes(GtkWidget *pButton, gpointer data);
void on_copier_button_clown(GtkWidget *pButton, gpointer data);
void on_copier_button_masque(GtkWidget *pButton, gpointer data);
void on_copier_button_jeuavecnez(GtkWidget *pButton, gpointer data);
void on_copier_button_screenshot(GtkWidget *pButton, gpointer data);
void on_copier_button_suivisvisage(GtkWidget *pButton, gpointer data);
void on_copier_buttonarret(GtkWidget *pButton, gpointer data);

//--------------------Déclaration des fonctions avec gtk
void ecriture(char lettre,int nb){
	for(int i=0;i<nb;i++){
    FILE* fp;
    printf("fichier\n");

    fp = fopen ("/dev/ttyACM0", "w");
    printf("fichier trouvé\n");

    if (fp == NULL) {
        printf("fichier introuvable : abandon\n");
    }

    else {
        fprintf(fp, "%c", lettre);
        printf("écrit : %c \n", lettre);
        //for(int j=0;j<99999999;j++){
          //wait
        //}
    }
    fclose(fp);
    printf("fichier fermé\n");
    }
}

//--------------------Dessine la ligne faite de plusieurs points : polyline
void dessineligne(cv::Mat &image, full_object_detection landmarks, int pointdebut, int pointfin, bool estferme){
    std::vector<cv::Point> points;
    int xx;
    int yy;

    for(int i=pointdebut; i<=pointfin; i++){
        //il y a 68 points de reference de 0 à 67. voir point.png pour + infos

        xx=landmarks.part(i).x();
        yy=landmarks.part(i).y();
        points.push_back(cv::Point(xx, yy));

        //point + gros sur les points importants a localiser qui nous servirrons à positionner les images en transparence
        if((i==36)||(i==39)||(i==42)||(i==45)||(i==29)||(i==33)||(i==48)||(i==51)||(i==54)||(i==57)){
            cv::circle(image,cv::Point(xx, yy),2,cv::Scalar(169, 79, 255), 2, 0);
            //circle(matrice cible, point centre, rayon, couleur BGR, épaisseur,flag)
        }
        if(flagsuivisvisage && i==30){
            cv::circle(image,cv::Point(xx, yy),4,cv::Scalar(0, 255, 139), 2, 0);
        }
    }
    cv::polylines(image, points, estferme, cv::Scalar(255, 255, 0), 1, 16);
}

//--------------------Dessine les pour le contour du visage
void dessinesleslignes(cv::Mat &image, full_object_detection landmarks){
    dessineligne(image, landmarks, 0, 16,false);              //ligne de la machoire 
    dessineligne(image, landmarks, 17, 21,false);             //sourcil gauche 
    dessineligne(image, landmarks, 22, 26,false);             //sourcil droit
    dessineligne(image, landmarks, 27, 30,false);             //ligne vertical nez
    dessineligne(image, landmarks, 30, 35, true);       //bas du nez
    dessineligne(image, landmarks, 36, 41, true);       //oeil gauche 
    dessineligne(image, landmarks, 42, 47, true);       //oeil droit 
    dessineligne(image, landmarks, 48, 59, true);       //levre exterieure 
    dessineligne(image, landmarks, 60, 67, true);       //levre intererieure 
}

//--------------------Trouve les points clefs du visage et dessines les lignes correspondantes
void landmarks_dessineleslignes(Mat &img_originale, frontal_face_detector faceDetector, shape_predictor landmarkDetector, 
    std::vector<dlib::rectangle> &faces, float resizeScale, int skipFrames, int frameCounter,Mat &img_neutre,
    int* dataFiltre1, int* dataFiltre2, int* dataFiltre3, int* dataFiltre4, int* dataFiltre5, int* dataFiltre6, int* dataFiltre7){
/*  Trouve les points landmark sur le visage et remplit en même temps les données nécessaires à l'application des filtres;
    Cette fonction sert à mettre à jour tous les données pertinentes en temps réel   */

        //pour stocker l'image resizee 
        Mat img_resizee;        

        //pour le resize de img_originale vers img_resizee
        resize(img_originale, img_resizee, Size(), 1.0/resizeScale, 1.0/resizeScale);

        //changer pour se mettre en format dlib image
        cv_image<bgr_pixel> dlibImageSmall(img_resizee);
        cv_image<bgr_pixel> dlibImage(img_originale);

        //detect faces a l'interval de skipFrames
        if(frameCounter % skipFrames == 0){
            faces = faceDetector(dlibImageSmall);
        }

        //boucle pour les visages // permet de dessiner les lignes pour plusieurs visages en même temps 
        int decale=25;
        nb_visages=faces.size();
        putText(img_originale, "Nombre de visages detectes " + to_string(nb_visages), cv::Point(10, 1*decale), FONT_HERSHEY_DUPLEX, 0.85, cv::Scalar(255, 255, 0), 1.8,8);

        //indique sur l'écran le filtre actif
        if(dataFiltre1[0]){
            putText(img_originale, "Filtre miroir droit actif", cv::Point(10, 3*decale), FONT_HERSHEY_DUPLEX, 0.8, cv::Scalar(0,255,255), 1.5,8);
        }
        if(dataFiltre2[0]){
            putText(img_originale, "Filtre miroir gauche actif", cv::Point(10, 4*decale), FONT_HERSHEY_DUPLEX, 0.8, cv::Scalar(0,255,255), 1.5,8);
        }
        if(dataFiltre3[0]){
            putText(img_originale, "Filtre dessin actif", cv::Point(10, 5*decale), FONT_HERSHEY_DUPLEX, 0.8, cv::Scalar(0,255,255), 1.5,8);
        }
        if(dataFiltre4[0]){
            putText(img_originale, "Filtre lunettes actif", cv::Point(10, 6*decale), FONT_HERSHEY_DUPLEX, 0.8, cv::Scalar(0,255,255), 1.5,8);
        }
        if(dataFiltre5[0]){
            putText(img_originale, "Filtre nez clown actif", cv::Point(10, 7*decale), FONT_HERSHEY_DUPLEX, 0.8, cv::Scalar(0,255,255), 1.5,8);
        }
        if(dataFiltre6[0]){
            putText(img_originale, "Filtre masque actif", cv::Point(10, 8*decale), FONT_HERSHEY_DUPLEX, 0.8, cv::Scalar(0,255,255), 1.5,8);
        }
        if(dataFiltre7[0]){
            putText(img_originale, "Filtre dessine avec nez actif", cv::Point(10, 9*decale), FONT_HERSHEY_DUPLEX, 0.8, cv::Scalar(0,255,255), 1.5,8);
        }

        if(flagtextlabel==1){
            putText(img_originale, "Filtre texte actif, activez un filtre", cv::Point(10, 10*decale), FONT_HERSHEY_DUPLEX, 0.8, cv::Scalar(0,255,255), 1.5,8);
            //putText(img_neutre, sText, cv::Point(50, 50), FONT_HERSHEY_DUPLEX, 1, cv::Scalar(0,0,255), 2,8);
        }
        if (flagsuivisvisage){
            putText(img_originale, "Suivis visage actif", cv::Point(10, 11*decale), FONT_HERSHEY_DUPLEX, 0.8, cv::Scalar(0,255,255), 1.5,8);
        }
        
        if(nb_visages==0){
            //si le visage venait a ne plus être reconnu, il faut reset la position du centre du visage 
            //sinon les servos en mode suivi visage continueraient de se déplacer indéfiniement  
            posX=320;   
            posY=240;
        }

        for(int i=0; i<faces.size(); i++){
            //determe les coordonnées du rectangle autour du visage a partir de l'image resizee 
            int ptgauche = int(faces[i].left() * resizeScale);
            int pthaut = int(faces[i].top() * resizeScale);
            int ptdroite = int(faces[i].right() * resizeScale);
            int ptbas = int(faces[i].bottom() * resizeScale);

            int largeur_rect = abs(ptdroite - ptgauche);
            int hauteur_rect = abs(ptbas - pthaut);

            dlib::rectangle rect(ptgauche,pthaut,ptdroite,ptbas);

            //detection des landmark du visage 
            full_object_detection faceLandmark = landmarkDetector(dlibImage, rect);

            //mise à jour des données d'entrée pour les filtres
            dataFiltre1[2]=faceLandmark.part(29).x();   //position du centre du visage
            dataFiltre2[2]=dataFiltre1[2];

            dataFiltre4[2]=faceLandmark.part(36).x();    //position des yeux
            dataFiltre4[3]=faceLandmark.part(36).y();
            dataFiltre4[4]=faceLandmark.part(45).x();
            dataFiltre4[5]=faceLandmark.part(45).y();

            dataFiltre6[2]=dataFiltre4[2];
            dataFiltre6[3]=dataFiltre4[3];
            dataFiltre6[4]=dataFiltre4[4];
            dataFiltre6[5]=dataFiltre4[5];

            dataFiltre5[2]=faceLandmark.part(30).x();    //position du nez
            dataFiltre5[3]=faceLandmark.part(30).y();
            dataFiltre5[4]=faceLandmark.part(33).x();
            dataFiltre5[5]=faceLandmark.part(33).y();

            dataFiltre7[2]=dataFiltre5[4];
            dataFiltre7[3]=dataFiltre5[5];

            posX=faceLandmark.part(30).x();
            posY=faceLandmark.part(30).y();

            //dessinesleslignes grace aux landmarks precedents 
            dessinesleslignes(img_originale, faceLandmark);
        }
}


//--------------------Premier filtre : image doublee droite en mode miroir
void filtre_miroirdroite(Mat &img_neutre,int suprmiroirdroite, int largeurmoitievisage, int flagscreenshot) 
{   
    cv::Mat img_totfiltredroite;
    cv::namedWindow( "img_totfiltredroite", WINDOW_AUTOSIZE);

    if(suprmiroirdroite==0 && nb_visages){
        //deuxieme filtre en mode couper visage partie droite et effet miroir
        int ptx_moitie2=maximumm(0,largeurmoitievisage);
        int pty_moitie2=0;
        int ptlargeur_moitie2=630-largeurmoitievisage;
        int pthauteur_moitie2=480;           

        cv::Mat img_moitievisage2;
        cv::Rect crop_regionmoitiedroite(ptx_moitie2,pty_moitie2,ptlargeur_moitie2,pthauteur_moitie2);    
        
        img_moitievisage2=img_neutre(crop_regionmoitiedroite);

        cv::Mat img_moitievisageflip2;
        cv::flip(img_moitievisage2,img_moitievisageflip2,1);
        
        cv::hconcat(img_moitievisageflip2,img_moitievisage2,img_totfiltredroite);
        cv::resize(img_totfiltredroite, img_totfiltredroite, cv::Size(img_totfiltredroite.cols * 0.91,img_totfiltredroite.rows * 0.91), 0, 0, cv::INTER_LINEAR);

        if(flagtextlabel==1){
            putText(img_totfiltredroite, sText, cv::Point(50, 50), FONT_HERSHEY_DUPLEX, 1, cv::Scalar(0,0,255), 2,8);
        }

        cv::imshow("img_totfiltredroite",img_totfiltredroite);
        cv::moveWindow("img_totfiltredroite",10,500);

        if (flagscreenshot) {   //si la fenêtre est active et le bouton screenshot actif, prendre une photo et la mettre dans le dossier photos
            imwrite("../photos/miroir_droite.png", img_totfiltredroite);
        }
    }
    else if (suprmiroirdroite==1){
        destroyWindow("img_totfiltredroite");
    }
}

//--------------------Deuxieme filtre : image doublee gauche en mode miroir
void filtre_miroirgauche(Mat &img_neutre,int suprmiroirgauche, int largeurmoitievisage, int flagscreenshot) 
{   
    cv::Mat img_totfiltregauche;
    cv::namedWindow( "img_totfiltregauche", WINDOW_AUTOSIZE);

    if(suprmiroirgauche==0 && nb_visages){
        //premier filtre en mode couper visage partie gauche et effet miroir
        cv::Mat img_moitievisage;
        cv::Rect crop_regionmoitiegauche(0,0,largeurmoitievisage, 480);    
        
        img_moitievisage=img_neutre(crop_regionmoitiegauche);

        cv::Mat img_moitievisageflip;
        cv::flip(img_moitievisage,img_moitievisageflip,1);
        
        cv::hconcat(img_moitievisage,img_moitievisageflip,img_totfiltregauche);
        cv::resize(img_totfiltregauche, img_totfiltregauche, cv::Size(img_totfiltregauche.cols * 0.91,img_totfiltregauche.rows * 0.91), 0, 0, cv::INTER_LINEAR);

        if(flagtextlabel==1){
            putText(img_totfiltregauche, sText, cv::Point(50, 50), FONT_HERSHEY_DUPLEX, 1, cv::Scalar(0,0,255), 2,8);
        }

        cv::imshow("img_totfiltregauche",img_totfiltregauche);
        cv::moveWindow("img_totfiltregauche",1000,500);

        if (flagscreenshot) {      //si la fenêtre est active et le bouton screenshot actif, prendre une photo et la mettre dans le dossier photos
            imwrite("../photos/miroir_gauche.png", img_totfiltregauche);
        }
    }
    else if (suprmiroirgauche==1){
        destroyWindow("img_totfiltregauche");
    }
}

//--------------------Troisieme filtre : image en mode dessin
void filtre_dessin(Mat &img_neutre, int suprfiltredessin, int flagscreenshot) 
{   
    cv::Mat img_detaillee;
    cv::Mat img_dessin;
    cv::namedWindow( "img_dessin", WINDOW_AUTOSIZE);

    if(suprfiltredessin==0 && nb_visages){        
        Mat imageBrighnessHighx; 
        img_neutre.convertTo(imageBrighnessHighx, -1, 1, 90); //augmente luminosite de x=...

        Mat img1;
        cv::detailEnhance(imageBrighnessHighx, img_detaillee, 10, 0.15f);
        cv::pencilSketch(img_detaillee,img1, img_dessin, 50 , 0.07f, 0.02f);

        cv::resize(img_dessin, img_dessin, cv::Size(img_dessin.cols * 0.91,img_dessin.rows * 0.91), 0, 0, cv::INTER_LINEAR);
        
        if(flagtextlabel==1){
            putText(img_dessin, sText, cv::Point(50, 50), FONT_HERSHEY_DUPLEX, 1, cv::Scalar(0,0,255), 2,8);
        }

        cv::imshow("img_dessin",img_dessin);
        cv::moveWindow("img_dessin",700,500);

        if (flagscreenshot) {      //si la fenêtre est active et le bouton screenshot actif, prendre une photo et la mettre dans le dossier photos
            imwrite("../photos/filtre_dessin.png", img_dessin);
        }
    }

    else if (suprfiltredessin==1){
        destroyWindow("img_dessin");
    }
}

//--------------------Quatrieme filtre : image avec lunettes
void filtre_lunettes(Mat &img_neutre, int suprlunettes, int XeyeG, int YeyeG, int XeyeD, int YeyeD, int flagscreenshot) {
/*  Filtre qui applique des lunettes sur le visage;
    la position des yeux est stické en temps réel dans le tableau DATA[4][2:5] */
    cv::Mat filtrelunettes;
    cv::Mat lunettes = imread("../source/glasses.png");
    cv::Mat lunettes_ech;
    cv::namedWindow ("img_filtrelunettes", WINDOW_AUTOSIZE);

    if(suprlunettes==0 && nb_visages){
        float ech = XeyeD-XeyeG;  //mise à l'échelle de l'image lunettes
        ech = 0.91*2*ech/LENlunettes;
        cv::resize(lunettes, lunettes_ech, Size(), ech, ech);

        int Lwid = lunettes_ech.rows;   int Llen = lunettes_ech.cols; //nouvelle taille de l'image
        int xAppInit = XeyeG-Llen/4; //coordonnées sur l'image originale
        int yAppInit = YeyeG-Lwid/2;

        cv::Vec3b pixelComp1 = lunettes.at<cv::Vec3b>(cv::Point (0,0)); //couleur du premier pixel transparent
        cv::Vec3b pixelComp2 = lunettes.at<cv::Vec3b>(cv::Point (2,2)); //OpenCV transforme les qui possèdent une chaîne alpha en une couleur blanche, alors on prendra quelques exemples pour les éliminer
        cv::Vec3b pixelComp3 = lunettes.at<cv::Vec3b>(cv::Point (Lwid,Llen));

        img_neutre.copyTo(filtrelunettes);
        for (int x=5; x<=Llen-5; x++) //copier les pixels non transparents
        for (int y=5; y<=Lwid-5; y++) {
            cv::Vec3b pixelCurr = lunettes_ech.at<cv::Vec3b>(cv::Point (x,y));
            int cx=xAppInit+x;  int cy=yAppInit+y;
            if ((pixelCurr!= pixelComp1)&&(pixelCurr != pixelComp2)&&(pixelCurr != pixelComp3)&&(cx<LENcam)&&(cy<WIDcam)&&(cx>0)&&(cy>0))   //la limite est la taille de la fenêtre
                filtrelunettes.at<Vec3b>(yAppInit+y,xAppInit+x) = lunettes_ech.at<Vec3b>(y,x);
        }

        cv::resize(filtrelunettes, filtrelunettes, cv::Size(filtrelunettes.cols * 0.91,filtrelunettes.rows * 0.91), 0, 0, cv::INTER_LINEAR);

        if(flagtextlabel==1){
            putText(filtrelunettes, sText, cv::Point(50, 50), FONT_HERSHEY_DUPLEX, 1, cv::Scalar(0,0,255), 2,8);
        }

        imshow ("img_filtrelunettes", filtrelunettes);
        cv::moveWindow("img_filtrelunettes",1050,0);

        if (flagscreenshot) {   //si la fenêtre est active et le bouton screenshot actif, prendre une photo et la mettre dans le dossier photos
            imwrite("../photos/avec_lunettes.png", filtrelunettes);
        }
    }

    else if (suprlunettes==1){
        destroyWindow("img_filtrelunettes");
    }
}

//--------------------Cinquieme filtre : image avec nez clown
void filtre_nezclown(Mat &img_neutre, int suprnezclown, int XnezH, int YnezH, int XnezB, int YnezB, int flagscreenshot) {
/*  Filtre qui applique un nez de clown sur le visage;
    la position du nez est stické en temps réel dans le tableau DATA[5][2:5]    */
    cv::Mat filtrenezclown;
    cv::Mat nezclown = imread("../source/clownnose.png");
    cv::Mat nezclown_ech;
    cv::namedWindow ("img_filtrenezclown", WINDOW_AUTOSIZE);

    if(suprnezclown==0 && nb_visages){
        float ech = YnezB-YnezH;  //mise à l'échelle de l'image nez clown
        ech = 0.91*4*ech/WIDnez;
        cv::resize(nezclown, nezclown_ech, Size(), ech, ech);

        int Lwid = nezclown_ech.rows;   //nouvelle taille de l'image
        int xAppInit = XnezB-Lwid/2;   //coordonnées sur l'image originale
        int yAppInit = YnezB-Lwid/2;

        cv::Vec3b pixelComp1 = nezclown_ech.at<cv::Vec3b>(cv::Point (0,0));   //couleur du premier pixel transparent
        cv::Vec3b pixelComp2 = nezclown_ech.at<cv::Vec3b>(cv::Point (5,5));
        cv::Vec3b pixelComp3 = nezclown_ech.at<cv::Vec3b>(cv::Point (Lwid,Lwid));

        img_neutre.copyTo(filtrenezclown);
        for (int x=0; x<=Lwid; x++) //copier les pixels non transparents
        for (int y=0; y<=Lwid-3; y++) {
            cv::Vec3b pixelCurr = nezclown_ech.at<cv::Vec3b>(cv::Point (x,y));
            int cx=xAppInit+x;  int cy=yAppInit+y;
            if ((pixelCurr!= pixelComp1)&&(pixelCurr != pixelComp2)&&(pixelCurr != pixelComp3)&&(cx<LENcam)&&(cy<WIDcam)&&(cx>0)&&(cy>0))
                filtrenezclown.at<Vec3b>(yAppInit+y,xAppInit+x) = nezclown_ech.at<Vec3b>(y,x);
        }
        cv::resize(filtrenezclown, filtrenezclown, cv::Size(filtrenezclown.cols * 0.91,filtrenezclown.rows * 0.91), 0, 0, cv::INTER_LINEAR);

        if(flagtextlabel==1){
            putText(filtrenezclown, sText, cv::Point(50, 50), FONT_HERSHEY_DUPLEX, 1, cv::Scalar(0,0,255), 2,8);
        }

        imshow ("img_filtrenezclown", filtrenezclown);

        if (flagscreenshot) {   //si la fenêtre est active et le bouton screenshot actif, prendre une photo et la mettre dans le dossier photos
            imwrite("../photos/nez_clown.png", filtrenezclown);
        }
    }

    else if (suprnezclown==1){
        destroyWindow("img_filtrenezclown");
    }
}

//--------------------Sixieme filtre : image avec masque
void filtre_masque(Mat &img_neutre, int suprmasque, int XeyeG, int YeyeG, int XeyeD, int YeyeD, int flagscreenshot) {
/*  Filtre qui applique un masque sur le visage;
    la position des yeux est stické en temps réel dans le tableau DATA[6][2:5]*/
    cv::Mat filtremasque;
    cv::Mat masque = imread("../source/masque2.png");
    cv::Mat masque_ech;
    cv::namedWindow ("img_filtremasque", WINDOW_AUTOSIZE);

    //disclaimer, toutes les positions sont finalement prises par rapport aux coordonnées des yeux car ceux ci sont plus stables

    if(suprmasque==0 && nb_visages){
        float ech = XeyeD-XeyeG;  //mise à l'échelle de l'image masque
        ech = 0.91*7*ech/(3*LENmasque); //definis de manière empirique

        cv::resize(masque, masque_ech, Size(), ech, ech);
        
        int Lwid = masque_ech.rows;   //nouvelle taille de l'image
        int Llen = masque_ech.cols;
        int xAppInit = XeyeG-2*Llen/7;   //coordonnées sur l'image originale
        int yAppInit = YeyeG+Lwid/9;
        
        cv::Vec3b pixelComp1 = masque.at<cv::Vec3b>(cv::Point (0,0));   //couleur du premier pixel transparent
        cv::Vec3b pixelComp2 = masque.at<cv::Vec3b>(cv::Point (5,5));

        img_neutre.copyTo(filtremasque);
        for (int x=0; x<=Llen-1; x++) //copier les pixels non transparents
        for (int y=0; y<=Lwid-1; y++) {
            cv::Vec3b pixelCurr = masque_ech.at<cv::Vec3b>(cv::Point (x,y));
            int cx=xAppInit+x;  int cy=yAppInit+y;
            if ((pixelCurr!= pixelComp1)&&(pixelCurr != pixelComp2)&&(cx<LENcam)&&(cy<WIDcam)&&(cx>0)&&(cy>0)){
                filtremasque.at<Vec3b>(cy,cx) = masque_ech.at<Vec3b>(y,x);
            }
        }

        cv::resize(filtremasque, filtremasque, cv::Size(filtremasque.cols * 0.91,filtremasque.rows * 0.91), 0, 0, cv::INTER_LINEAR);

        if(flagtextlabel==1){
            putText(filtremasque, sText, cv::Point(50, 50), FONT_HERSHEY_DUPLEX, 1, cv::Scalar(0,0,255), 2,8);
        }


        imshow ("img_filtremasque", filtremasque);

        if (flagscreenshot) {   //si la fenêtre est active et le bouton screenshot actif, prendre une photo et la mettre dans le dossier photos
            imwrite("../photos/avec_masque.png", filtremasque);
        }
    }
    else if (suprmasque==1){
        destroyWindow("img_filtremasque");
    }
}

cv::Mat imageavecligne; //variable globale - permet de nettoyer la fenêtre une fois elle est fermée

//---------------------Septième filtre : jeu dessinne avec nez
void filtre_jeudessineavecnez(Mat &img_neutre, int suprfiltrejeudessineavecnez, int Xnez, int Ynez, int* XnezPrec, int* YnezPrec, int flagscreenshot){
    //pendant 20 secondes le joeur peut dessiner sur l'image en temps réel en fonction de 
    //la position de son nez 
    //au bout des 20 secondes, le filtre est réinitialisé 
    cv::namedWindow ("img_filtrejeuavecnez", WINDOW_AUTOSIZE);
    cv:: Mat filtrejeuavecnez;
    
    if (suprfiltrejeudessineavecnez==0 && nb_visages){
        
        if(imageavecligne.empty()){
            imageavecligne = cv::Mat::zeros(img_neutre.size(), img_neutre.type());
        }

        GaussianBlur(img_neutre, filtrejeuavecnez, Size(7, 7), 5,0); 
        
        //dessine une ligne rouge entre l'ancien et le nouveau point de la position du nez
        cv::line(imageavecligne, Point(Xnez, Ynez), Point(*XnezPrec, *YnezPrec), Scalar(0,0,255), 3,LINE_AA,0);

        *XnezPrec=Xnez;
        *YnezPrec=Ynez;

        if(flagtextlabel==1){
            putText(filtrejeuavecnez, sText, cv::Point(50, 50), FONT_HERSHEY_DUPLEX, 1, cv::Scalar(0,0,255), 2,8);
        }

        imshow ("img_filtrejeuavecnez", filtrejeuavecnez + imageavecligne);
        cv::moveWindow("img_filtrejeuavecnez",300,500);

        if (flagscreenshot) {   //si la fenêtre est active et le bouton screenshot actif, prendre une photo et la mettre dans le dossier photos
            imwrite("../photos/dessin_nez.png", filtrejeuavecnez + imageavecligne);
        }
    }
    else if (suprfiltrejeudessineavecnez==1){
        destroyWindow("img_filtrejeuavecnez");
    }
}


int main(int argc,char **argv){
    //---------------------selection du module camera
    int val_index_cam =0;
    printf("Renseignez le *numero* de la camera a selectionner: ");
    
    //scanf("%d", &val_index_cam);
    if( scanf("%d", &val_index_cam) != 1 ) 
    {
    fprintf( stderr, "renseignez un numero uniquement  pour le scanf\n");
    exit(1);
    }

    //creation de l'objet videocapture qui recupère directement la video de la webcam en temps reel
    VideoCapture videoCapture(val_index_cam);

    //verification de l'ouverture de la webcam
    if(!videoCapture.isOpened()){
        cout<<"can not open webcam"<<endl;
        printf("renseignez un index correct de camera\n");
        printf("afin de connaitre les camera connectee:\n"); 
        printf(">> sudo apt-get install v4l-utils\n");
        printf(">> sudo v4l2-ctl --list-devices\n");
        return 0;
    }


    /*  Système des pointeurs nécessaires pour le fonctionnement des filtres;
        Ici, ils vont contenir les flag d'activation et de suppression, ainsi que les coordonnées en pixels des points d'intérêt;
        Chaque "colonne" corrspondera au bouton en question et les "lignes" seront occupés par les données nécessaires à l'activation de ce bouton;
        Les deux premières lignes seront occupés respéctivement par les flag d'activation et désactivation, les lignes suivantes sont les champs de données libres;
        Pour plus d'information se référer à DATA.png    */
    int DATA[NUMflags][NUMdata];
    for (int i=0; i<NUMflags; i++)
    for (int j=0; j<NUMdata; j++)   DATA[i][j]=0;   //tous les flag et les données sont initialisés à 0

    //déclaration du timer qui nous servira pour le dernier filtre
    time_t tempsJeu;
    time_t tempsFin;

    //---------------------PARTIE DETECTION VISAGE
    //definition du face detector
    frontal_face_detector faceDetector = get_frontal_face_detector();

    //definition du landmark detector
    shape_predictor landmarkDetector;
    
    //permet de charger le fichier modèle pour le face landmark
    deserialize("../shape_predictor_68_face_landmarks.dat") >> landmarkDetector;

    //definition de la hauteur/taille cible pour le resize
    float resize_hauteur = 480;

    //definition du skip frames //a ajuster en fonction de la fludité voulue
    int frameCounter=0;
    int skipFrames = 2;

    //Recupère premier frame de la webcam
    //defintion de la matrice cible 
    Mat img_originale;
    //initialisation
    videoCapture >> img_originale;
    cv::flip(img_originale,img_originale,1);//effet miroir selon avec vertical

    Mat img_neutre;
    videoCapture >> img_neutre;
    cv::flip(img_neutre,img_neutre,1);

    Mat img_suivis;
    videoCapture >> img_suivis;
    cv::flip(img_suivis,img_suivis,1);


    //calcule le facteur pour le resize
    float height = img_originale.rows;
    float resizeScale = height/resize_hauteur;

    //creation de la fenetre pour afficher chaque frame de la webcam
    namedWindow("img_originale", WINDOW_AUTOSIZE);
    //resizeWindow("img_originale",800,600);
    
    //definition pour maintenir les visages detectes 
    std::vector<dlib::rectangle> faces;


    //--------------------------------------------------------------------
    //PARTIE GTK
    GtkWidget* pWindow;
    GtkWidget *pVBox;
    GtkWidget* pLabel;

    GtkWidget *pButtonmiroirdroit;
    GtkWidget *pButtonmiroirgauche;
    GtkWidget *pButtondessin;
    GtkWidget *pButtonlunettes;
    GtkWidget *pButtonclown;
    GtkWidget *pButtonmasque;
    GtkWidget *pButtonquitter;
    GtkWidget *pButtonjeuavecnez;
    GtkWidget *pBouttonscreenshot;
    GtkWidget *pButtonsuivisvisage;

    GtkWidget *pEntry;
    GtkWidget *pResult;
    //GtkWidget *pImage;
    
    // Initialisation de GTK
    gtk_init(&argc,&argv);

    // Création de la fenetre principale (titre et taille de la fenetre)
    pWindow = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(pWindow), "Selection de l'effet");
    gtk_window_set_default_size(GTK_WINDOW(pWindow), 320, 200);
    gtk_window_move(GTK_WINDOW(pWindow), img_originale.cols, 0);
    
    // Connexion du signal de fermeture de la fenetre au fait de quitter le programme
    g_signal_connect(G_OBJECT(pWindow), "destroy", G_CALLBACK(gtk_main_quit), NULL);

    // Creation d'une verticale box qu'on ajoute à la fenetre principale
    // Attention ! La fenetre principale GTK ne peut contenir qu'un seul widget
    // Il faut ensuite utiliser des vbox et des hbox pour positionner les widgets à l'intérieur
    pVBox = gtk_vbox_new(TRUE, 0);    
    gtk_container_add(GTK_CONTAINER(pWindow), pVBox);
    
    
    /* Creation des labels */
    // Attention, le text du label doit etre converti en utf-8 si il comporte des caractères accentués, en utilisant : g_locale_to_utf8()
    pLabel=gtk_label_new("Label :");
    pResult = gtk_label_new(" ");
    
    /* Creation du GtkEntry */
    pEntry = gtk_entry_new();
    
    /*Creation des boutons */
    pButtonmiroirdroit = gtk_button_new_with_label("Filtre miroir droite");
    pButtonmiroirgauche = gtk_button_new_with_label("Filtre miroir gauche");
    pButtondessin = gtk_button_new_with_label("Filtre dessin");
    pButtonlunettes = gtk_button_new_with_label("Filtre lunettes");
    pButtonclown = gtk_button_new_with_label("Filtre nez clown");
    pButtonmasque = gtk_button_new_with_label("Filtre masque");
    pButtonjeuavecnez = gtk_button_new_with_label("Jeu dessinne avec nez");
    pBouttonscreenshot = gtk_button_new_with_label("Photo!");
    pButtonsuivisvisage = gtk_button_new_with_label("Suivi visage");
    pButtonquitter = gtk_button_new_with_label("Quitter");

    //Creation image
    //pImage = gtk_image_new_from_file("../polytech.jpg");
    
    /* Insertion des widgets dans la GtkVBox */
    // L'ordre d'insertion des éléments dans la vBox correspond à l'ordre d'apparition des éléments dans la fenetre
    gtk_box_pack_start(GTK_BOX(pVBox), pLabel, TRUE, FALSE, 0);   
    gtk_box_pack_start(GTK_BOX(pVBox), pEntry, TRUE, FALSE, 0);

    gtk_box_pack_start(GTK_BOX(pVBox), pButtonmiroirdroit, TRUE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(pVBox), pButtonmiroirgauche, TRUE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(pVBox), pButtondessin, TRUE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(pVBox), pButtonlunettes, TRUE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(pVBox), pButtonclown, TRUE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(pVBox), pButtonmasque, TRUE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(pVBox), pButtonjeuavecnez, TRUE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(pVBox), pBouttonscreenshot, TRUE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(pVBox), pButtonsuivisvisage, TRUE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(pVBox), pButtonquitter, TRUE, FALSE, 0);

    gtk_box_pack_start(GTK_BOX(pVBox), pResult, TRUE, FALSE, 0);
    //gtk_box_pack_start(GTK_BOX(pVBox), pImage, FALSE, FALSE, 0);
    
    /* Connexion du signal "activate" du GtkEntry */
    g_signal_connect(G_OBJECT(pEntry), "activate", G_CALLBACK(on_activate_entry), (GtkWidget*) pLabel);

    /* Connexion du signal "clicked" du GtkButton */
    g_signal_connect(G_OBJECT(pButtonmiroirdroit), "clicked", G_CALLBACK(on_copier_button_miroirdroit), DATA[1]);   
    //les données modifiés par la fonction callback seront stockés dans le tableau à une dimension pointé par DATA[i], i corréspondant au numéro du bouton/filtre
    g_signal_connect(G_OBJECT(pButtonmiroirgauche), "clicked", G_CALLBACK(on_copier_button_miroirgauche), DATA[2]);
    g_signal_connect(G_OBJECT(pButtondessin), "clicked", G_CALLBACK(on_copier_button_dessin), (GtkWidget*) DATA[3]);
    g_signal_connect(G_OBJECT(pButtonlunettes), "clicked", G_CALLBACK(on_copier_button_lunettes), (GtkWidget*) DATA[4]);
    g_signal_connect(G_OBJECT(pButtonclown), "clicked", G_CALLBACK(on_copier_button_clown), (GtkWidget*) DATA[5]);
    g_signal_connect(G_OBJECT(pButtonmasque), "clicked", G_CALLBACK(on_copier_button_masque), (GtkWidget*) DATA[6]);
    g_signal_connect(G_OBJECT(pButtonjeuavecnez), "clicked", G_CALLBACK(on_copier_button_jeuavecnez), (GtkWidget*) DATA[7]);
    g_signal_connect(G_OBJECT(pBouttonscreenshot), "clicked", G_CALLBACK(on_copier_button_screenshot), (GtkWidget*) DATA[8]);
    g_signal_connect(G_OBJECT(pButtonsuivisvisage), "clicked", G_CALLBACK(on_copier_button_suivisvisage), (GtkWidget*) pVBox);
    g_signal_connect(G_OBJECT(pButtonquitter), "clicked", G_CALLBACK(on_copier_buttonarret), (GtkWidget*) DATA[0]);

    /* Affichage de la fenêtré et de tout ce qu'il contient */
    gtk_widget_show_all(pWindow);

    /* Demarrage de la boucle évenementielle */
    //gtk_main();

    //--------------------------------------------------------------------
    //BOUCLE INFINIE TRAITEMENT VIDEO et lancement des filtres
    while (1){
        //lecture de la webcam et stockage dans Matrice img_originale
        videoCapture >> img_originale;
        cv::flip(img_originale,img_originale,1);//effet miroir selon avec vertical

        videoCapture >> img_neutre;
        cv::flip(img_neutre,img_neutre,1);

        videoCapture >> img_suivis;
        cv::flip(img_suivis,img_suivis,1);

        landmarks_dessineleslignes  (img_originale, faceDetector, landmarkDetector, faces, resizeScale, skipFrames, frameCounter,img_neutre,
                                    DATA[1], DATA[2], DATA[3], DATA[4], DATA[5], DATA[6], DATA[7]);
        //cv::imshow("visageisole",img1crop);

        //--------------------------------------------------------------------
        //suivis de visage
        char shaut='w';
        char sbas='s';
        char sgauche='a'; 
        char sdroite='d';
        int keyValeur = cv::waitKey(1) & 0xFF;
        
        if(keyValeur==114){//r = reset position des servos
            ecriture('r',1);
		}

        cv::namedWindow("img_suivis", cv::WINDOW_AUTOSIZE);
        cv::moveWindow("img_suivis",1000,0);

        if(flagsuivisvisage){
                cv::rectangle(img_suivis,cv::Point(320-DELTA,0),cv::Point(320+DELTA,480), cv::Scalar( 255, 0, 0 )); //cadrillage sur la vidéo pour mieux se reperer
		        cv::rectangle(img_suivis,cv::Point(0,240-DELTA),cv::Point(640,240+DELTA), cv::Scalar( 255, 0, 0 )); //idem

                cv::circle(img_suivis, cv::Point(posX, posY), 3, cv::Scalar(0, 255, 139), 3); //point au centre de l'objet

                if(keyValeur==114){//r = reset position des servos
                    ecriture('r',1);
				}

                if ((posX>320-DELTA)&&(posX<320+DELTA)&&(posY>0)&&(posY<240-DELTA)) { //carre haut centre
					cv::rectangle(img_suivis,cv::Point(320-DELTA,0),cv::Point(320+DELTA,240-DELTA), cv::Scalar( 23, 10, 255 ),2);
					ecriture(shaut,1);
				}
				else if ((posX>320-DELTA)&&(posX<320+DELTA)&&(posY>240+DELTA)&&(posY<480)) {  //carre bas centre
					cv::rectangle(img_suivis,cv::Point(320-DELTA,240+DELTA),cv::Point(320+DELTA,480), cv::Scalar( 23, 10, 255 ),2);
					ecriture(sbas,1);
				}
				else if ((posX>0)&&(posX<320-DELTA)&&(posY>240-DELTA)&&(posY<240+DELTA)) { //carre gauche centre
					cv::rectangle(img_suivis,cv::Point(0,240-DELTA),cv::Point(320-DELTA,240+DELTA), cv::Scalar( 23, 10, 255 ),2);
					ecriture(sgauche,2);
				}
				else if ((posX>320+DELTA)&&(posX<640)&&(posY>240-DELTA)&&(posY<240+DELTA)) { //carre droite centre
					cv::rectangle(img_suivis,cv::Point(320+DELTA,240-DELTA),cv::Point(640,240+DELTA), cv::Scalar( 23, 10, 255 ),2);
					ecriture(sdroite,2);
				}
				else if ((posX<320-DELTA)&&(posY<240-DELTA)) { //carre haut gauche
					cv::rectangle(img_suivis,cv::Point(0,0),cv::Point(320-DELTA,240-DELTA), cv::Scalar( 0, 0, 255 ),2);
					ecriture(shaut,2);
					ecriture(sgauche,1);
				}
				else if ((posX>320+DELTA)&&(posY<240-DELTA)) { //carre haut droite
					cv::rectangle(img_suivis,cv::Point(320+DELTA,0),cv::Point(640,240-DELTA), cv::Scalar( 0, 0, 255 ),2);
					ecriture(shaut,2);
					ecriture(sdroite,1);
				}
				else if ((posX<320-DELTA)&&(posY>240+DELTA)) { //carre bas gauche
					cv::rectangle(img_suivis,cv::Point(0,240+DELTA),cv::Point(320-DELTA,480), cv::Scalar( 0, 0, 255 ),2);
					ecriture(sbas,2);
					ecriture(sgauche,1);
				}
				else if ((posX>320+DELTA)&&(posY>240+DELTA)) { //carre bas droite
					cv::rectangle(img_suivis,cv::Point(320+DELTA,240+DELTA),cv::Point(640,480), cv::Scalar( 0, 0, 255 ),2);
					ecriture(sbas,2);
					ecriture(sdroite,1);
				}
                cv::imshow("img_suivis", img_suivis);
        }
        if(!flagsuivisvisage){ //si le suivis de visage est désactivé on n'affiche pas la fenêtre
            destroyWindow("img_suivis");
        }


        //--------------------------------------------------------------------
        //premier filtre miroir droite
        if(DATA[1][0]){
            filtre_miroirdroite(img_neutre,DATA[1][1],DATA[1][2],DATA[8][0]);
        }
        if ((!DATA[1][0])&&(DATA[1][1])){
            //permet d'appeller la fonction et de destroy la fenetre 
            filtre_miroirdroite(img_neutre,DATA[1][1],DATA[1][2],DATA[8][0]);
        }

        //deuxieme filtre miroir gauche
        if(DATA[2][0]){
            filtre_miroirgauche(img_neutre,DATA[2][1],DATA[2][2],DATA[8][0]);
        }
        if ((!DATA[2][0])&&(DATA[2][1])){
            //permet d'appeler la fonction et de destroy la fenetre 
            filtre_miroirgauche(img_neutre,DATA[2][1],DATA[2][2],DATA[8][0]);
        }

        //troisieme filtre dessin
        if(DATA[3][0]){
            filtre_dessin(img_neutre,DATA[3][1],DATA[8][0]);
        }
        if ((!DATA[3][0])&&(DATA[3][1])){
            //permet d'appeler la fonction et de destroy la fenetre 
            filtre_dessin(img_neutre,DATA[3][1],DATA[8][0]);
        }

        //quatrième filtre lunettes
        if(DATA[4][0]){
            filtre_lunettes(img_neutre,DATA[4][1],DATA[4][2],DATA[4][3],DATA[4][4],DATA[4][5],DATA[8][0]);
        }
        if ((!DATA[4][0])&&(DATA[4][1])){
            //permet d'appeler la fonction et de destroy la fenetre 
            filtre_lunettes(img_neutre,DATA[4][1],DATA[4][2],DATA[4][3],DATA[4][4],DATA[4][5],DATA[8][0]);
        }

        //cinquieme filtre nez clown
        if(DATA[5][0]){
            filtre_nezclown(img_neutre,DATA[5][1],DATA[5][2],DATA[5][3],DATA[5][4],DATA[5][5],DATA[8][0]);
        }
        if ((!DATA[5][0])&&(DATA[5][1])){
            //permet d'appeler la fonction et de destroy la fenetre 
            filtre_nezclown(img_neutre,DATA[5][1],DATA[5][2],DATA[5][3],DATA[5][4],DATA[5][5],DATA[8][0]);
        }

        //sixieme filtre nez clown
        if(DATA[6][0]){
            filtre_masque(img_neutre,DATA[6][1],DATA[6][2],DATA[6][3],DATA[6][4],DATA[6][5],DATA[8][0]);
        }
        if ((!DATA[6][0])&&(DATA[6][1])){
            //permet d'appeler la fonction et de destroy la fenetre 
            filtre_masque(img_neutre,DATA[6][1],DATA[6][2],DATA[6][3],DATA[6][4],DATA[6][5],DATA[8][0]);
        }

        //septieme filtre jeu avec nez
        if(DATA[7][0]){
            if (DATA[7][6]) {
                tempsJeu = time(NULL);
                printf("Début des 20 secondes\n");
                DATA[7][6]=0;
            }
            filtre_jeudessineavecnez(img_neutre,DATA[7][1],DATA[7][2],DATA[7][3],&DATA[7][4],&DATA[7][5],DATA[8][0]);
            tempsFin = time(NULL);
            if (difftime(tempsFin,tempsJeu)>20) {
                imageavecligne = cv::Mat::zeros(img_neutre.size(), img_neutre.type()); //reset des lignes
                printf("Fin des 20 secondes, reset des lignes\n");
                tempsJeu = time(NULL);
            }
        }
        if ((!DATA[7][0])&&(DATA[7][1])){
            //permet d'appeler la fonction et de destroy la fenetre 
            filtre_jeudessineavecnez(img_neutre,DATA[7][1],DATA[7][2],DATA[7][3],&DATA[7][4],&DATA[7][5],DATA[8][0]);
            imageavecligne = cv::Mat::zeros(img_neutre.size(), img_neutre.type()); //reset des lignes
        }

        //--------------------------------------------------------------------

        //affichage de img_originale
        cv::resize(img_originale, img_originale, cv::Size(img_originale.cols * 0.91,img_originale.rows * 0.91), 0, 0, cv::INTER_LINEAR);
        cv::imshow("img_originale", img_originale);
        cv::moveWindow("img_originale",10,0);

        //--------------------------------------------------------------------
        if(DATA[0][0]){
                //dlib::sleep(100);
                std::cout << R"(
                          _      ____  _            _        _     _ 
                         / \    | __ )(_) ___ _ __ | |_ ___ | |_  | |
                        / _ \   |  _ \| |/ _ \ '_ \| __/ _ \| __| | |
                       / ___ \  | |_) | |  __/ | | | || (_) | |_  |_|
                      /_/   \_\ |____/|_|\___|_| |_|\__\___/ \__| (_)

             _ __ ___   ___ _ __ ___(_)   __| | ( )   __ ___   _____ (_)_ __ 
            | '_ ` _ \ / _ \ '__/ __| |  / _` | |/   / _` \ \ / / _ \| | '__|
            | | | | | |  __/ | | (__| | | (_| |     | (_| |\ V / (_) | | |   
            |_| |_| |_|\___|_|  \___|_|  \__,_|      \__,_| \_/ \___/|_|_|   
                        _   _ _ _                        _            
                  _   _| |_(_) (_)___  ___   _ __   ___ | |_ _ __ ___ 
                 | | | | __| | | / __|/ _ \ | '_ \ / _ \| __| '__/ _ \
                 | |_| | |_| | | \__ \  __/ | | | | (_) | |_| | |  __/
                  \__,_|\__|_|_|_|___/\___| |_| |_|\___/ \__|_|  \___|                                                       
                                     _ _           _   _             
                    __ _ _ __  _ __ | (_) ___ __ _| |_(_) ___  _ __  
                   / _` | '_ \| '_ \| | |/ __/ _` | __| |/ _ \| '_ \ 
                  | (_| | |_) | |_) | | | (_| (_| | |_| | (_) | | | |
                   \__,_| .__/| .__/|_|_|\___\__,_|\__|_|\___/|_| |_|
                        |_|   |_|                                    

                 ____             _  _               _     ____       _       
                | __ )  ___ _ __ (_)| |_ ___     ___| |_  |  _ \ ___ | | ___  
                |  _ \ / _ \ '_ \| || __/ _ \   / _ \ __| | |_) / _ \| |/ _ \ 
                | |_) |  __/ | | | || || (_) | |  __/ |_  |  __/ (_) | | (_) |
                |____/ \___|_| |_|_| \__\___/   \___|\__| |_|   \___/|_|\___/ 
                                                                    
                )" << '\n';
                
                destroyAllWindows();
                exit(EXIT_SUCCESS);
        }

        DATA[8][0]=0;   //remettre le flag de prise des photos à 0 une fois que tous les photos seront prises


        //Pour quitter le programme au choix: touche echap ou q 
        char key = waitKey(1);
        if( (key == 27) || (key == 113) ){
            break;
        }
    }

    //release de l'objet video capture
    videoCapture.release();

    //ferme toutes les fenetres
    destroyAllWindows();
    
    return 0;
}


//--------------------FONCTIONS POUR GTK
/* Fonction callback execute lors du signal "activate" */
void on_activate_entry(GtkWidget *pEntry, gpointer data)
{
    //const gchar *sText;
    /* Recuperation du texte contenu dans le GtkEntry */
    sText = gtk_entry_get_text(GTK_ENTRY(pEntry));
    /* Modification du texte contenu dans le GtkLabel */
    gtk_label_set_text(GTK_LABEL((GtkWidget*)data), sText);
    flagtextlabel=!flagtextlabel;
}

/* Fonction callback executee lors du signal "clicked" */
void on_copier_button_miroirdroit(GtkWidget *pButton, gpointer data) {
    int* DATA = (int*) data;    //on transforme gpointer en int* - tableau à une dimension
    if (!DATA[0])   DATA[1]=0;  //mise à jour du flag de suppréssion
    else            DATA[1]=1;
    DATA[0]=!DATA[0];   //mise à jour du flag d'activation
}

void on_copier_button_miroirgauche(GtkWidget *pButton, gpointer data)
{
    int* DATA = (int*) data;
    if (!DATA[0])   DATA[1]=0;
    else            DATA[1]=1;
    DATA[0]=!DATA[0];
}

void on_copier_button_dessin(GtkWidget *pButton, gpointer data)
{
    int* DATA = (int*) data;
    if (!DATA[0])   DATA[1]=0;
    else            DATA[1]=1;
    DATA[0]=!DATA[0];
}

void on_copier_button_lunettes(GtkWidget *pButton, gpointer data)
{
    int* DATA = (int*) data;
    if (!DATA[0])   DATA[1]=0;
    else            DATA[1]=1;
    DATA[0]=!DATA[0];
}

void on_copier_button_clown(GtkWidget *pButton, gpointer data)
{
    int* DATA = (int*) data;
    if (!DATA[0])   DATA[1]=0;
    else            DATA[1]=1;
    DATA[0]=!DATA[0];
}

void on_copier_button_masque(GtkWidget *pButton, gpointer data)
{
    int* DATA = (int*) data;
    if (!DATA[0])   DATA[1]=0;
    else            DATA[1]=1;
    DATA[0]=!DATA[0];
}

void on_copier_button_jeuavecnez(GtkWidget *pButton, gpointer data)
{
    int* DATA = (int*) data;
    if (!DATA[0])   DATA[1]=0;
    else            DATA[1]=1;
    DATA[0]=!DATA[0];
    DATA[6]=1;  //activation du timer
}

void on_copier_button_screenshot(GtkWidget *pButton, gpointer data) //active mode screenshot qui enregistre en format png toutes les filtres et donc fenetres actives
{
    int* DATA = (int*) data;
    DATA[0]=!DATA[0];
}

void on_copier_button_suivisvisage(GtkWidget *pButton, gpointer data)
{
    flagsuivisvisage=!flagsuivisvisage;
}


//Fonction pour le bouton d'arret
void on_copier_buttonarret(GtkWidget *pButton, gpointer data)
{   
    int* DATA = (int*) data;
    DATA[0]=1;
}
