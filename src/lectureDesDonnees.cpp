// Projet final - lecture des données et insnertion dans tables SQL
// Samuel Doucette - A00216008
// Alec Jones - A00216262

#include <iostream>
#include <fstream>
#include <stdexcept>
#include <sstream>
#include <string>
#include <ocilib.h>
using namespace std;

void ouvreFichier(fstream &fichier, string nom);
void creerTables(OCI_Connection *conn);
void effacerTables(OCI_Connection *conn);
void traiterFichierProgrammes(OCI_Connection *conn);
void traiterFichierRepertoire(OCI_Connection *conn);
void traiterFichierEvaluations(OCI_Connection *conn);
void traiterFichierEchelle(OCI_Connection *conn);

int main(){
    try{
        OCI_EnableWarnings(true);


        // Initialiser OCILIB
        if(!OCI_Initialize(NULL, NULL, OCI_ENV_DEFAULT))
            throw runtime_error("Erreur avec l'initialisation de l'environnement OCILIB.");

        // Se conencter à la base de données
        string nomUtilisateur, motDePasse;
        cout << "Nom d'utilisateur : ";
        cin >> nomUtilisateur;
        cout << "Mot de passe : ";
        cin >> motDePasse;

        OCI_Connection *conn = OCI_ConnectionCreate("xe", const_cast<char*>(nomUtilisateur.c_str()), const_cast<char*>(motDePasse.c_str()), OCI_SESSION_DEFAULT);
        if(!conn) 
            throw runtime_error("Connexion a la base de donnees a echouee.");

        //Effacer les tables
        effacerTables(conn);

        //Créer les tables
        creerTables(conn);

        // Traiter les fichiers CSV
        traiterFichierProgrammes(conn);
        traiterFichierRepertoire(conn);
        traiterFichierEvaluations(conn);
        traiterFichierEchelle(conn);

        // Nettoyage
        OCI_ConnectionFree(conn);
        OCI_Cleanup();
    }
    catch (const exception &e){
        cerr << "Erreur : " << e.what() << endl;
    }

    return 0;
}

// Ouvrir le fichier csv
void ouvreFichier(fstream &fichier, string nom){
    fichier.open(nom, ios::in);

    if (!fichier.is_open())
        throw runtime_error("Pas capable d'ouvrir le fichier " + nom);
}

void creerTables(OCI_Connection *conn){
    OCI_Statement *createSt = OCI_StatementCreate(conn);

    // Table Programmes
    OCI_ExecuteStmt(createSt, "create table Programmes(ID numeric(3), Titre varchar2(128), Credits numeric(3), primary key(ID))");

    // Table Repertoire
    OCI_ExecuteStmt(createSt, "create table Repertoire(Sigle char(8), Titre varchar2(128), Credits numeric(2), primary key(Sigle))");
    
    // Table CoursExiges
    OCI_ExecuteStmt(createSt, "create table CoursExiges(ID numeric(3), Sigle char(8), primary key(ID, Sigle), foreign key(ID) references Programmes(ID), foreign key(Sigle) references Repertoire(Sigle))");

    // Table Etudiants
    OCI_ExecuteStmt(createSt, "create table Etudiants(NI char(9), Nom char(128), ID numeric(3), primary key(NI), foreign key(ID) references Programmes(ID))");

    // Table CoursOfferts
    OCI_ExecuteStmt(createSt, "create table CoursOfferts(Semestre char(3), Sigle char(8), primary key(Semestre, Sigle), foreign key(Sigle) references Repertoire(Sigle))");

    // Table Inscriptions
    OCI_ExecuteStmt(createSt, "create table Inscriptions(NI char(9), Semestre char(3), Sigle char(8), primary key(NI, Semestre, Sigle), foreign key(NI) references Etudiants(NI), foreign key(Semestre, Sigle) references CoursOfferts(Semestre, Sigle))");

    // Table Evaluations
    OCI_ExecuteStmt(createSt, "create table Evaluations(Sigle char(8), NomEval varchar2(128), Poids numeric(3), primary key(Sigle, NomEval), foreign key(Sigle) references Repertoire(Sigle))");

    // Table Notes
    OCI_ExecuteStmt(createSt, "create table Notes(NI char(9), Semestre char(3), Sigle char(8), NomEval varchar2(128), Points numeric(3), primary key(NI, Semestre, Sigle, NomEval), foreign key(NI) references Etudiants(NI), foreign key(Semestre, Sigle) references CoursOfferts(Semestre, Sigle), foreign key(Sigle, NomEval) references Evaluations(Sigle, NomEval))");

    // Table EchelleNotes
    OCI_ExecuteStmt(createSt, "create table EchelleNotes(Points numeric(3), Lettre varchar2(2), Note numeric(3,2), primary key(Points))");

    OCI_StatementFree(createSt);

    OCI_Commit(conn);
}

void effacerTables(OCI_Connection *conn){
    OCI_Statement *dropSt = OCI_StatementCreate(conn);

    // Table EchelleNotes
    OCI_ExecuteStmt(dropSt, "drop table EchelleNotes cascade constraints");

    // Table Notes
    OCI_ExecuteStmt(dropSt, "drop table Notes cascade constraints");
    
    // Table Evaluations
    OCI_ExecuteStmt(dropSt, "drop table Evaluations cascade constraints");

    // Table Inscriptions
    OCI_ExecuteStmt(dropSt, "drop table Inscriptions cascade constraints");

    // Table CoursOfferts
    OCI_ExecuteStmt(dropSt, "drop table CoursOfferts cascade constraints");

    // Table Etudiants
    OCI_ExecuteStmt(dropSt, "drop table Etudiants cascade constraints");

    // Table CoursExiges
    OCI_ExecuteStmt(dropSt, "drop table CoursExiges cascade constraints");

    // Table Repertoire
    OCI_ExecuteStmt(dropSt, "drop table Repertoire cascade constraints");

    // Table Programmes
    OCI_ExecuteStmt(dropSt, "drop table Programmes cascade constraints");

    OCI_StatementFree(dropSt);

    OCI_Commit(conn);
}

void traiterFichierProgrammes(OCI_Connection *conn){
    // Ouvrir le fichier programmes.csv
    fstream fichier;
    ouvreFichier(fichier, "data/programmes.csv");

    // Lire et insérer chaque ligne à l'intérieur de la table programme
    string ligne;
    bool ligneDEnTete = true;
    while (getline(fichier, ligne)){
        // (Sauter la ligne d'en-tête)
        if(ligneDEnTete){
            ligneDEnTete = false;
            continue;
        }

        istringstream ligneStream(ligne);
        string idString, titre, creditsString;

        // Séparer les champs
        getline(ligneStream, idString, ';');
        getline(ligneStream, titre, ';');
        getline(ligneStream, creditsString, ';');

        // Convertir Id et Credits en int
        int id = stoi(idString);
        int credits = stoi(creditsString);

        // Insérer la ligne dans la table
        OCI_Statement *insertSt = OCI_CreateStatement(conn);
        OCI_Prepare(insertSt, "INSERT INTO Programmes(ID, Titre, Credits) VALUES (:ID, :Titre, :Credits)");
        OCI_BindInt(insertSt, ":ID", &id);
        OCI_BindString(insertSt, ":Titre", const_cast<char*>(titre.c_str()), 0);
        OCI_BindInt(insertSt, ":Credits", &credits);
        OCI_Execute(insertSt);
        OCI_StatementFree(insertSt);
    }

    // Appliquer les insertions
    OCI_Commit(conn);

    // Fermer le fichier
    fichier.close();
}

void traiterFichierRepertoire(OCI_Connection *conn){
    // Ouvrir le fichier repertoire.csv
    fstream fichier;
    ouvreFichier(fichier, "data/repertoire.csv");

    // Lire et insérer chaque ligne à l'intérieur de la table Repertoire
    string ligne;
    bool ligneDEnTete = true;
    while (getline(fichier, ligne)){
        // (Sauter la ligne d'en-tête)
        if(ligneDEnTete){
            ligneDEnTete = false;
            continue;
        }

        istringstream ligneStream(ligne);
        string programme, sigle, titre, creditsString, sessions, session;

        // Séparer les champs (1er niveau)
        getline(ligneStream, programme, ';');
        getline(ligneStream, sigle, ';');
        getline(ligneStream, titre, ';');
        getline(ligneStream, creditsString, ';');
        getline(ligneStream, sessions, ';');

        // Convertir Credits en int
        int credits = stoi(creditsString);

        // Insérer la ligne dans la table repertoire
        OCI_Statement *insertSt = OCI_CreateStatement(conn);
        OCI_Prepare(insertSt, "INSERT INTO Repertoire(Sigle, Titre, Credits) VALUES (:Sigle, :Titre, :Credits)");
        // OCI_BindString(insertSt, ":Programme", const_cast<char*>(programme.c_str()), 0);
        OCI_BindString(insertSt, ":Sigle", const_cast<char*>(sigle.c_str()), 0);
        OCI_BindString(insertSt, ":Titre", const_cast<char*>(titre.c_str()), 0);
        OCI_BindInt(insertSt, ":Credits", &credits);
        OCI_Execute(insertSt);
        
        // Pour chaque session dans la ligne, faire un insertion dans la table CoursOfferts
        istringstream sessionsStream(sessions);
        while(getline(sessionsStream, session, ',')){
            OCI_Prepare(insertSt, "INSERT INTO CoursOfferts(Semestre, Sigle) VALUES (:Semestre, :Sigle)");
            OCI_BindString(insertSt, ":Semestre", const_cast<char*>(session.c_str()), 0);
            OCI_BindString(insertSt, ":Sigle", const_cast<char*>(sigle.c_str()), 0);
            OCI_Execute(insertSt);
        }

        OCI_StatementFree(insertSt);
    }

    // Appliquer les insertions
    OCI_Commit(conn);

    // Fermer le fichier
    fichier.close();

}

void traiterFichierEvaluations(OCI_Connection *conn){
    // Ouvrir le fichier evaluations.csv
    fstream fichier;
    ouvreFichier(fichier, "data/evaluations.csv");

    // Lire et insérer chaque ligne à l'intérieur de la table Evaluations
    string ligne;
    bool ligneDEnTete = true;
    while (getline(fichier, ligne)){
            // (Sauter la ligne d'en-tête)
            if(ligneDEnTete){
                ligneDEnTete = false;
                continue;
            }

            istringstream ligneStream(ligne);
            string sigle, evalPoids, evaluation, poidsString;

            // Séparer les champs (1er niveau)
            getline(ligneStream, sigle, ';');
            getline(ligneStream, evalPoids, ';');

            // Pour chaque evaluation dans la ligne, faire un insertion
            istringstream evalPoidsStream(evalPoids);
            while(getline(evalPoidsStream, evaluation, ',')){
                getline(evalPoidsStream, poidsString, ',');
                
                // Convertir Poids en int
                int poids = stoi(poidsString);

                // Insérer la ligne dans la table
                OCI_Statement *insertSt = OCI_CreateStatement(conn);
                OCI_Prepare(insertSt, "INSERT INTO Evaluations(Sigle, NomEval, Poids) VALUES (:Sigle, :NomEval, :Poids)");
                OCI_BindString(insertSt, ":Sigle", const_cast<char*>(sigle.c_str()), 0);
                OCI_BindString(insertSt, ":NomEval", const_cast<char*>(evaluation.c_str()), 0);
                OCI_BindInt(insertSt, ":Poids", &poids);
                OCI_Execute(insertSt);
                OCI_StatementFree(insertSt);
            }  
    }

     // Appliquer les insertions
     OCI_Commit(conn);

     // Fermer le fichier
     fichier.close();
}

void traiterFichierEchelle(OCI_Connection *conn){
    // Ouvrir le fichier echelle.csv
    fstream fichier;
    ouvreFichier(fichier, "data/echelle.csv");

    // Lire et insérer chaque ligne à l'intérieur de la table EchelleNotes
    string ligne;
    bool ligneDEnTete = true;
    while (getline(fichier, ligne)){
            // (Sauter la ligne d'en-tête)
            if(ligneDEnTete){
                ligneDEnTete = false;
                continue;
            }

            istringstream ligneStream(ligne);
            string pointsString, lettre, noteString;

            // Séparer les champs
            getline(ligneStream, pointsString, ';');
            getline(ligneStream, lettre, ';');
            getline(ligneStream, noteString, ';');

            // Convertir Points et Note en int
            int points = stoi(pointsString);
            int note = stoi(noteString);

            // Insérer la ligne dans la table
            OCI_Statement *insertSt = OCI_CreateStatement(conn);
            OCI_Prepare(insertSt, "INSERT INTO EchelleNotes(Points, Lettre, Note) VALUES (:Points, :Lettre, :Note)");
            OCI_BindInt(insertSt, ":Points", &points);
            OCI_BindString(insertSt, ":Lettre", const_cast<char*>(lettre.c_str()), 0);
            OCI_BindInt(insertSt, ":Note", &note);
            OCI_Execute(insertSt);
            OCI_StatementFree(insertSt);
   }

   // Appliquer les insertions
   OCI_Commit(conn);

   // Fermer le fichier
   fichier.close();
}


/*
    À FAIRE:
        S'assurer que le code fonctionne
        Commencer à écrire le rapport (au moins ma partie)
        Reviser l'énoncée, s'assurer que j'oublie rien.
*/