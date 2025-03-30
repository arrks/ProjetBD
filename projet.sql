-- INFO4030 - Hiver 2025 - Projet "Mini Socrate"
-- Descriptions des tables

create table Programmes(
	ID numeric(3),
	Titre varchar2(128),
	Credits numeric(3),
	primary key(ID));

create table Repertoire(
	Sigle char(8),
	Titre varchar2(128),
	Credits numeric(2),
	primary key(Sigle));

create table CoursExiges(
	ID numeric(3),
	Sigle char(8),
	primary key(ID, Sigle),
	foreign key(ID) references Programmes(ID),
	foreign key(Sigle) references Repertoire(Sigle));

create table Etudiants(
	NI char(9),
	Nom char(128),
	ID numeric(3),
	primary key(NI),
	foreign key(ID) references Programmes(ID));

create table CoursOfferts(
	Semestre char(3),
	Sigle char(8),
	primary key(Semestre, Sigle));

create table Inscriptions(
	NI char(9),
	Semestre char(3),
	Sigle char(8),
	primary key(NI, Semestre, Sigle),
	foreign key(NI) references Etudiants(NI),
	foreign key(Semestre, Sigle) references CoursOfferts(Semestre, Sigle));

create table Evaluations(
	Sigle char(8),
	NomEval varchar2(128),
	Poids numeric(3),
	primary key(Sigle, NomEval),
	foreign key(Sigle) references Repertoire(Sigle));

create table Notes(
	NI char(9),
	Semestre char(3),
	Sigle char(8),
	NomEval varchar2(128),
	Points numeric(3),
	primary key(NI, Semestre, Sigle, NomEval),
	foreign key(NI) references Etudiants(NI),
	foreign key(Semestre, Sigle) references CoursOfferts(Semestre, Sigle),
	foreign key(Sigle, NomEval) references Evaluations(Sigle, NomEval));

create table EchelleNotes(
	Points numeric(3),
	Lettre varchar2(2),
	Note numeric(3,2),
	primary key(Points));

-- Pour effacer toutes les tables :
drop table EchelleNotes cascade constraints;
drop table Notes cascade constraints;
drop table Evaluations cascade constraints;
drop table Inscriptions cascade constraints;
drop table CoursOfferts cascade constraints;
drop table Etudiants cascade constraints;
drop table CoursExiges cascade constraints;
drop table Repertoire cascade constraints;
drop table Programmes cascade constraints;
