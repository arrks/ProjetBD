// Projet final - Programme d'interface à la base de données
// Samuel Doucette - A00216008
// Alec Jones - A00216262

#include <iostream>
#include <string>
#include <limits>
#include <cctype>
#include <ocilib.h>
using namespace std;

char normaliseInput(string input);
void AfficherDisplayBuffer(OCI_Connection *conn);
string prompt();
string prompt(string promptText);
void listeProgrammes(OCI_Connection *conn);
void listeCoursParSession(OCI_Connection *conn);
void inscrireEtudiant(OCI_Connection *conn);
void inscrireEtudiantCours(OCI_Connection *conn);
void ajouterNote(OCI_Connection *conn);
void calculerNoteFinale(OCI_Connection *conn);
void calculerMoyenneCumulative(OCI_Connection *conn);

void errorHandler(OCI_Error *err){
    // Traiter les codes d'erreur

    if (!err) {
        cerr << "Erreur système" << endl;
        return;
    }

    int errorCode = OCI_ErrorGetOCICode(err);
    
    switch(errorCode) {
        case 20001:
            cerr << "Étudiant non trouvé" << endl;
            break;
        case 20002:
            cerr << "Évaluation non trouvée" << endl;
            break;
        case 20003:
            cerr << "La note existe déjà pour l'étudiant au cours" << endl;
            break;
        case 20004:
            cerr << "L'étudiant est déjà inscrit au cours" << endl;
            break;
        case 20005:
            cerr << "L'étudiant est déjà inscrit au programme" << endl;
            break;
        case 20006:
            cerr << "Le programme n'existe pas" << endl;
            break;
        case 20007:
            cerr << "Le cours n'existe pas" << endl;
            break;
        case 20008:
            cerr << "L'étudiant n'est pas inscrit au cours" << endl;
            break;
        case 20009:
            cerr << "Il manque des notes pour l'étudiant au cours" << endl;
            break;
        default:
            cerr << "Erreur de base de données" << endl;
    }
}

int main(int argc, char *argv[]){
    try{
        OCI_EnableWarnings(true);

        // Initialiser OCILIB
        if(!OCI_Initialize(errorHandler, NULL, OCI_ENV_DEFAULT))
            throw runtime_error("Erreur avec l'initialisation de l'environnement OCILIB.");

        // Connexion à la base de données
        string nomUtilisateur, motDePasse;
        if (argc >= 3) {
            nomUtilisateur = argv[1];
            motDePasse = argv[2];
        } else {
            cout << "Nom d'utilisateur : ";
            cin >> nomUtilisateur;
            cout << "Mot de passe : ";
            cin >> motDePasse;
            cout << endl;
        }

        OCI_Connection *conn = OCI_ConnectionCreate("xe", const_cast<char*>(nomUtilisateur.c_str()), const_cast<char*>(motDePasse.c_str()), OCI_SESSION_DEFAULT);
        if(!conn) 
            throw runtime_error("Connexion a la base de donnees a echouee.");

        // Enable output
        OCI_ServerEnableOutput(conn, 32000, 5, 255);

        // Page principale
        string userInput;
        cout << "====================\nBienvenue sur Mini Socrate!\n====================" << endl;
        cout << "Pour obtenir la liste de commandes, appuyez sur (H)." << endl << endl;
    
        while(true){
            // Lire la commande entrée
            userInput = prompt();
    
            // Exécuter la commande
            switch(normaliseInput(userInput)){
            case 'H':
                cout << "Voici la liste de commandes disponibles : " << endl
                    << "(P) - Afficher la liste des programmes, en ordre alphabetique." << endl
                    << "(C) - Afficher la liste des cours par session, en ordre alphabetique." << endl
                    << "(E) - S'inscrire comme nouvel.le etudiant.e a un programme existant ou bien se desinscrire." << endl
                    << "(I) - S'inscrire a un cours d'une certaine session ou bien se desinscrire." << endl
                    << "(N) - Ajouter ou supprimer une note obtenue a l'evaluation d'un cours a une session donnee." << endl
                    << "(F) - Calculer la note finale pour un cours d'une session donnee, si toutes les notes sont entrees." << endl
                    << "(M) - Calculer la moyenne cumulative d'une etudiante ou d'un etudiant." << endl
                    << "(H) - Afficher la liste de commandes disponibles." << endl
                    << "(X) - Sortir du programme." << endl;
            break;
            case 'X':
                cout << "Terminaison du programme.\n====================" << endl << endl;
                return 0;
            break;
            case 'P':
                listeProgrammes(conn);
                break;
            case 'C':
                listeCoursParSession(conn);
                break;
            case 'E':
                inscrireEtudiant(conn);
                break;
            case 'I':
                inscrireEtudiantCours(conn);
                break;
            case 'N':
                ajouterNote(conn);
                break;
            case 'F':
                calculerNoteFinale(conn);
                break;
            case 'M':
                calculerMoyenneCumulative(conn);
                break;
            default:
                cout << "Erreur: commande (" << userInput << ") n'est pas reconnue.\nPour obtenir une liste de commandes, appuyez sur (H)." << endl;
                break;
            }
            
            cout << "--------------------" << endl << endl;

            OCI_Commit(conn);
        }

        // Nettoyage
        OCI_ConnectionFree(conn);
        OCI_Cleanup();
    }
    catch (const exception &e){
        cerr << "Erreur : " << e.what() << endl;
    }

    return 0;
}

char normaliseInput(string input){
    // Déterminer s'il y a plus qu'un charactère dans l'entrée
    char output = ' ';
    for(char i: input){
        if(output == ' '){
            output = i;
        }
        else{
            return '!';
        }
    }

    // Remettre le charactère en majuscule.
    return toupper(output);
}

void AfficherDisplayBuffer(OCI_Connection *conn){
    // Afficher le diplay buffer de OCILIB
    const char *p;
    while((p = OCI_ServerGetOutput(conn)) != NULL){
        printf("%s", p);
        printf("\n");
    }
}

string prompt(){
    // Demander pour une commande à l'utilisateur
    string userInput;
    cout << "> ";
    cin >> userInput;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    
    return userInput;
}

string prompt(string promptText){
    // Demander pour une commande à l'utilisateur
    string userInput;
    cout << promptText << endl;
    cout << "> ";
    cin >> userInput;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    
    return userInput;
}

void listeProgrammes(OCI_Connection *conn){
    OCI_Statement *executeSt = OCI_StatementCreate(conn);

    // Executer la procedure
    if (OCI_ExecuteStmt(executeSt, "BEGIN liste_programmes; END;")) {   
        cout << "Programmes :\n--------------------" << endl;
        AfficherDisplayBuffer(conn);
    }

    OCI_StatementFree(executeSt);
}

void listeCoursParSession(OCI_Connection *conn){
    OCI_Statement *executeSt = OCI_StatementCreate(conn);

    // Demander la session
    string session;
    session = prompt("Choisissez la session que vous voulez afficher les cours : ");

    string script = "BEGIN liste_cours_par_session('" + session + "'); END;";

    // Executer la procedure
    if(OCI_ExecuteStmt(executeSt, const_cast<char*>(script.c_str()))){ 
        cout << "Cours pour la session " << session << " :\n--------------------" << endl;
        AfficherDisplayBuffer(conn);
    }

    OCI_StatementFree(executeSt);
}

void inscrireEtudiant(OCI_Connection *conn){
    OCI_Statement *executeSt = OCI_StatementCreate(conn);

    // Demander si on veut inscrire ou désinscrire
    string actionString = prompt("Voulez-vous inscrire un etudiant a un programme (I) ou desinscrire un etudiant a un programme (D)?");
    char action = normaliseInput(actionString);

    string NI, programmeID, nom, script;
    
    // Executer la procedure
    switch(action){
    case 'I':   // Inscription
        // Demander pour les paramètres
        cout << "Inscription a un programme. Veuillez fournir l'information suivante : " << endl;
        NI = prompt("NI : ");
        programmeID = prompt("ID du programme : ");
        nom = prompt("Prenom et Nom : ");

        script = "BEGIN inscrire_etudiant('" + NI + "', " + programmeID + ", 'i', '" + nom + "'); END;";

        if(OCI_ExecuteStmt(executeSt, const_cast<char*>(script.c_str()))){
            AfficherDisplayBuffer(conn);
        }

        break;


    case 'D':   // Désinscription
        // Demander pour les paramètres
        cout << "Desinscription a un programme. Veuillez fournir l'information suivante : " << endl;
        NI = prompt("NI : ");
        programmeID = prompt("ID du programme : ");

        script = "BEGIN inscrire_etudiant('" + NI + "', " + programmeID + ", 'd'); END;";

        if(OCI_ExecuteStmt(executeSt, const_cast<char*>(script.c_str()))){
            AfficherDisplayBuffer(conn);
        }

        break;

    default:    // Option inconnue
        cout << "(" << actionString << ") n'est pas une option reconnue.\nVeuillez inscrire (I) ou (D)." << endl;
        break;
    }

    OCI_StatementFree(executeSt);
}

void inscrireEtudiantCours(OCI_Connection *conn){
    OCI_Statement *executeSt = OCI_StatementCreate(conn);

    // Demander si on veut inscrire ou désinscrire
    string actionString = prompt("Voulez-vous inscrire un etudiant a un cours (I) ou desinscrire un etudiant a un cours (D)?");
    char action = normaliseInput(actionString);

    string NI, sigle, session, script;
    
    // Executer la procedure
    switch(action){
    case 'I':   // Inscription
        // Demander pour les paramètres
        cout << "Inscription a un cours. Veuillez fournir l'information suivante : " << endl;
        NI = prompt("NI : ");
        sigle = prompt("Sigle du cours : ");
        session = prompt("Session (format A21) : ");

        script = "BEGIN inscrire_etudiant_cours('" + NI + "', '" + sigle + "', '" + session +  "', 'i'); END;";

        if(OCI_ExecuteStmt(executeSt, const_cast<char*>(script.c_str()))){
            AfficherDisplayBuffer(conn);
        }

        break;


    case 'D':   // Désinscription
        // Demander pour les paramètres
        cout << "Desnscription a un cours. Veuillez fournir l'information suivante : " << endl;
        NI = prompt("NI : ");
        sigle = prompt("Sigle du cours : ");
        session = prompt("Session (format A21): ");

        script = "BEGIN inscrire_etudiant_cours('" + NI + "', '" + sigle + "', '" + session +  "', 'd'); END;";

        if(OCI_ExecuteStmt(executeSt, const_cast<char*>(script.c_str()))){
            AfficherDisplayBuffer(conn);
        }

        break;

    default:    // Option inconnue
        cout << "(" << actionString << ") n'est pas une option reconnue.\nVeuillez inscrire (I) ou (D)." << endl;
        break;
    }

    OCI_StatementFree(executeSt);
}

void ajouterNote(OCI_Connection *conn){
    OCI_Statement *executeSt = OCI_StatementCreate(conn);

    // Demander si on veut ajouter ou enlever une note
    string actionString = prompt("Voulez-vous inscrire une note d'evaluation pour un etudiant (A) ou enlever une note d'evaluation pour un etudiant (E)?");
    char action = normaliseInput(actionString);

    string NI, sigle, session, nomEval, note, script;
    
    // Executer la procedure
    switch(action){
    case 'A':   // Ajouter
        // Demander pour les paramètres
        cout << "Ajout d'une note. Veuillez fournir l'information suivante : " << endl;
        NI = prompt("NI : ");
        sigle = prompt("Sigle du cours : ");
        session = prompt("Session (format A21) : ");
        cout << "Nom de l'evaluation : \n> ";
        getline(cin, nomEval);
        note = prompt("Note : ");

        script = "BEGIN ajouter_note('" + NI + "', '" + sigle + "', '" + session + "', '" + nomEval + "', '" + note + "'); END;";

        if(!OCI_ExecuteStmt(executeSt, const_cast<char*>(script.c_str()))){
            const otext *err = OCI_ErrorGetString(OCI_GetLastError());
            cerr << "Erreur lors de l'execution de la procedure ajouter_note (ajout) : " << (err ? err : "Erreur inconnue") << endl;
        } else {
            AfficherDisplayBuffer(conn);
        }

        break;

    case 'E':   // Désinscription
        // Demander pour les paramètres
        cout << "Enlever une note. Veuillez fournir l'information suivante : " << endl;
        NI = prompt("NI : ");
        sigle = prompt("Sigle du cours : ");
        session = prompt("Session (format A21) : ");
        cout << "Nom de l'evaluation : \n> ";
        getline(cin, nomEval);

        script = "BEGIN ajouter_note('" + NI + "', '" + sigle + "', '" + session + "', '" + nomEval + "'); END;";

        if(!OCI_ExecuteStmt(executeSt, const_cast<char*>(script.c_str()))){
            const otext *err = OCI_ErrorGetString(OCI_GetLastError());
            cerr << "Erreur lors de l'execution de la procedure ajouter_note (enlever) : " << (err ? err : "Erreur inconnue") << endl;
        } else {
            AfficherDisplayBuffer(conn);
        }

        break;

    default:    // Option inconnue
        cout << "(" << actionString << ") n'est pas une option reconnue.\nVeuillez inscrire (A) ou (E)." << endl;
        break;
    }

    OCI_StatementFree(executeSt);
}

void calculerNoteFinale(OCI_Connection *conn){
    OCI_Statement *executeSt = OCI_StatementCreate(conn);

    // Demander la session
    string NI, sigle, session;
    cout << "Calculer la note finale. Veuillez fournir l'information suivante :" << endl;
    NI = prompt("NI : ");
    sigle = prompt("Sigle : ");
    session = prompt("Session (format A21) : ");

    string script = "BEGIN calculer_note_finale('" + NI + "', '" + sigle + "', '" + session + "'); END;";

    // Executer la procedure
    if(OCI_ExecuteStmt(executeSt, const_cast<char*>(script.c_str()))){
        AfficherDisplayBuffer(conn);
    }

    OCI_StatementFree(executeSt);
}

void calculerMoyenneCumulative(OCI_Connection *conn){
    OCI_Statement *executeSt = OCI_StatementCreate(conn);

    // Demander le NI
    string NI;
    cout << "Calculer la moyenne cumulative. Veuillez fournir l'information suivant : " << endl;
    NI = prompt("NI : ");

    string script = "BEGIN calculer_moyenne_cumulative('" + NI + "'); END;";

    // Executer la procedure
    if(OCI_ExecuteStmt(executeSt, const_cast<char*>(script.c_str()))){
        AfficherDisplayBuffer(conn);
    }

    OCI_StatementFree(executeSt);
}