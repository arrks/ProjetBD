-- Document de procédures à être exécutées par le menu cpp.
-- Les procédures seront stockées dans la base de données mais le code restera dans ce fichier pour documentation.

-- Procédure 1: Liste de programmes
-- Description: Cette procédure permet de lister les programmes disponibles dans la base de données en ordre alphabétique par ID.
-- Utilisation: EXECUTE liste_programmes;
create or replace procedure liste_programmes is
   cursor c_programmes is
   select ID
     from programmes
    order by ID;
   v_ID programmes.ID%type;
begin
   open c_programmes;
   loop
      fetch c_programmes into v_ID;
      exit when c_programmes%NOTFOUND;
      DBMS_OUTPUT.PUT_LINE(v_ID);
   end loop;
   close c_programmes;
end liste_programmes;

-- Procédure 2: Afficher la liste de cours par session, en ordre des sigles
-- Description: Cette procédure permet d'afficher la liste des cours offerts par session, triée par sigle.
-- Utilisation: EXECUTE liste_cours_par_session;
create or replace procedure liste_cours_par_session is
   cursor c_cours is
   select sigle,
          semestre
     from CoursOfferts
    order by semestre,
             sigle;
   v_sigle    CoursOfferts.sigle%type;
   v_semestre CoursOfferts.semestre%type;
begin
   open c_cours;
   loop
      fetch c_cours into
         v_sigle,
         v_semestre;
      exit when c_cours%NOTFOUND;
      DBMS_OUTPUT.PUT_LINE(v_semestre
                           || ' - '
                           || v_sigle);
   end loop;
   close c_cours;
end liste_cours_par_session;

-- Procédure 3: S'inscrire comme nouvel étudiant
-- Description: Cette procédure permet à un nouvel étudiant de s'inscrire à un programme existant ou se désinscrire d'un programme.
-- Utilisation: EXECUTE inscrire_etudiant (p_NI, p_nom, p_programme, p_action);
-- p_action peut être 'inscrire' ou 'desinscrire'.
create or replace procedure inscrire_etudiant (
   p_NI        in Etudiants.NI%type,
   p_nom       in Etudiants.Nom%type,
   p_programme in Etudiants.ID%type,
   p_action    in varchar2
) is
   v_programme Etudiants.ID%type;
   v_numero    Etudiants.NI%type;
begin
   -- Vérifier si l'étudiant existe
   select NI
     into v_numero
     from Etudiants
    where NI = p_NI;

   -- Vérifier si le programme existe
   select ID
     into v_programme
     from Programmes
    where ID = p_programme;

   if p_action = 'inscrire' then
      insert into Inscription (
         NI,
         nom,
         ID
      ) values ( p_NI,
                 p_nom,
                 p_programme );
      DBMS_OUTPUT.PUT_LINE('Inscription réussie pour l''étudiant '
                           || p_NI
                           || ' au programme '
                           || p_programme);
   elsif p_action = 'desinscrire' then
      delete from Inscription
       where NI = p_NI
         and IDProgramme = p_programme;
      DBMS_OUTPUT.PUT_LINE('Désinscription réussie pour l''étudiant '
                           || p_NI
                           || ' du programme '
                           || p_programme);
   else
      DBMS_OUTPUT.PUT_LINE('Action non reconnue. Utilisez ''inscrire'' ou ''desinscrire''.');
   end if;
exception
   when no_data_found then
      DBMS_OUTPUT.PUT_LINE('Erreur: Étudiant ou programme non trouvé.');
   when others then
      DBMS_OUTPUT.PUT_LINE('Erreur: ' || sqlerrm);
end inscrire_etudiant;

-- Procédure 4: Inscrire un étudiant à un cours ou le désinscrire
-- Description: Cette procédure permet d'inscrire un étudiant à un cours ou de le désinscrire d'un cours.
-- Utilisation: EXECUTE inscrire_etudiant_cours (p_NI, p_sigle, p_semestre, p_action);
-- p_action peut être 'inscrire' ou 'desinscrire'.
create or replace procedure inscrire_etudiant_cours (
   p_NI       in Etudiants.NI%type,
   p_sigle    in CoursOfferts.sigle%type,
   p_semestre in CoursOfferts.semestre%type,
   p_action   in varchar2
) is
   v_sigle    CoursOfferts.sigle%type;
   v_semestre CoursOfferts.semestre%type;
   v_numero   Etudiants.NI%type;
begin
   -- Vérifier si l'étudiant existe
   select NI
     into v_numero
     from Etudiants
    where NI = p_NI;

   -- Vérifier si le cours existe
   select sigle,
          semestre
     into
      v_sigle,
      v_semestre
     from CoursOfferts
    where sigle = p_sigle
      and semestre = p_semestre;

   if p_action = 'inscrire' then
      insert into InscriptionCours (
         NI,
         sigle,
         semestre
      ) values ( p_NI,
                 p_sigle,
                 p_semestre );
      DBMS_OUTPUT.PUT_LINE('Inscription réussie pour l''étudiant '
                           || p_NI
                           || ' au cours '
                           || p_sigle);
   elsif p_action = 'desinscrire' then
      delete from InscriptionCours
       where NI = p_NI
         and sigle = p_sigle
         and semestre = p_semestre;
      DBMS_OUTPUT.PUT_LINE('Désinscription réussie pour l''étudiant '
                           || p_NI
                           || ' du cours '
                           || p_sigle);
   else
      DBMS_OUTPUT.PUT_LINE('Action non reconnue. Utilisez ''inscrire'' ou ''desinscrire''.');
   end if;
exception
   when no_data_found then
      DBMS_OUTPUT.PUT_LINE('Erreur: Étudiant ou cours non trouvé.');
   when others then
      DBMS_OUTPUT.PUT_LINE('Erreur: ' || sqlerrm);
end inscrire_etudiant_cours;

-- Procédure 5: Ajouter ou supprimer la note obtenue
-- Description: Cette procédure permet d'ajouter ou de supprimer la note obtenue par un étudiant à une évaluation d'un cours à une session donnée.
-- Utilisation: EXECUTE ajouter_note (p_NI, p_sigle, p_semestre, p_nomeval p_note);
-- pour supprimer laisser p_note vide.
create or replace procedure ajouter_note (
   p_NI       in notes.NI%type,
   p_sigle    in notes.sigle%type,
   p_semestre in notes.semestre%type,
   p_nomeval  in notes.Nomeval%type,
   p_note     in notes.points%type default null
) is
   v_NI       notes.NI%type;
   v_sigle    notes.sigle%type;
   v_semestre notes.semestre%type;
   v_nomeval  notes.Nomeval%type;
begin
   -- Vérifier si l'étudiant existe
   select NI
     into v_NI
     from Etudiants
    where NI = p_NI;

   -- Vérifier si l'évaluation existe
   select sigle,
          nomeval,
          into v_sigle,
          v_nomeval
     from Evaluations
    where sigle = p_sigle
      and nomeval = p_nomeval;

   if p_note is not null then
      insert into notes (
         NI,
         sigle,
         semestre,
         Nomeval,
         points
      ) values ( p_NI,
                 p_sigle,
                 p_semestre,
                 p_nomeval,
                 p_note );
      DBMS_OUTPUT.PUT_LINE('Note ajoutée pour l''étudiant '
                           || p_NI
                           || ' au cours '
                           || p_sigle);
   else
      delete from notes
       where NI = p_NI
         and sigle = p_sigle
         and semestre = p_semestre;
      DBMS_OUTPUT.PUT_LINE('Note supprimée pour l''étudiant '
                           || p_NI
                           || ' au cours '
                           || p_sigle);
   end if;
exception
   when no_data_found then
      DBMS_OUTPUT.PUT_LINE('Erreur: Étudiant ou évaluation non trouvé.');
   when others then
      DBMS_OUTPUT.PUT_LINE('Erreur: ' || sqlerrm);
end ajouter_note;

-- Procédure 6: Calculer la note finale
-- Description: Cette procédure permet de calculer la note finale d'un étudiant pour un cours à une session donnée si toutes
-- les notes sont présentes à l'aide des poids définis dans la table Evaluations.
-- Utilisation: EXECUTE calculer_note_finale (p_NI, p_sigle, p_semestre);
create or replace procedure calculer_note_finale (
   p_NI       in notes.NI%type,
   p_sigle    in notes.sigle%type,
   p_semestre in notes.semestre%type
) is
   v_NI          notes.NI%type;
   v_sigle       notes.sigle%type;
   v_semestre    notes.semestre%type;
   v_note        number;
   v_total       number := 0;
   v_poids       number := 0;
   v_poids_total number := 0;
begin
   -- Vérifier si l'étudiant existe
   select NI
     into v_NI
     from Etudiants
    where NI = p_NI;

   -- Vérifier si le cours existe
   select sigle,
          semestre
     into
      v_sigle,
      v_semestre
     from CoursOfferts
    where sigle = p_sigle
      and semestre = p_semestre;

   -- Calculer la note finale
   for r in (
      select Nomeval,
             points,
             poids
        from notes n,
             Evaluations e
       where n.Nomeval = e.Nomeval
         and n.sigle = e.sigle
         and n.semestre = e.semestre
         and n.NI = p_NI
   ) loop
      v_total := v_total + ( r.points * r.poids );
      v_poids_total := v_poids_total + r.poids;
   end loop;

   if v_poids_total > 0 then
      v_note := v_total / v_poids_total;
      DBMS_OUTPUT.PUT_LINE('La note finale pour l''étudiant '
                           || p_NI
                           || ' au cours '
                           || p_sigle
                           || ' est: '
                           || v_note);
   else
      DBMS_OUTPUT.PUT_LINE('Aucune note trouvée pour l''étudiant ' || p_NI);
   end if;
exception
   when no_data_found then
      DBMS_OUTPUT.PUT_LINE('Erreur: Étudiant ou cours non trouvé.');
   when others then
      DBMS_OUTPUT.PUT_LINE('Erreur: ' || sqlerrm);
end calculer_note_finale;

-- Procédure 7: 