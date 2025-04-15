# Paramètres de connexion à la base de données (à mettre à jour si nécessaire)
-include config.mk
DB_USER ?= votre_nom_utilisateur
DB_PASSWORD ?= votre_mot_de_passe
DB_CONN = localhost:1521/xe

# Paramètres du compilateur
CXX = g++
LIBS = -locilib

.PHONY: clean import run_lecture run_menu setup

import:
	sqlplus $(DB_USER)/$(DB_PASSWORD)@$(DB_CONN) @scripts/procedures.plsql

lecture: src/lectureDesDonnees.cpp
	$(CXX) -o lecture.out src/lectureDesDonnees.cpp $(LIBS)

menu: src/menu.cpp
	$(CXX) -o menu.out src/menu.cpp $(LIBS)

run_lecture: lecture.out
	@./lecture.out $(DB_USER) $(DB_PASSWORD)

run_menu: menu.out
	@./menu.out $(DB_USER) $(DB_PASSWORD)

setup:
	$(MAKE) import
	$(MAKE) lecture
	$(MAKE) run_lecture
	$(MAKE) clean
	@echo "Configuration terminée."

clean:
	rm -f lecture menu *.out