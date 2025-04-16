# Paramètres de connexion à la base de données
-include config.mk
DB_USER ?= votre_nom_utilisateur
DB_PASSWORD ?= votre_mot_de_passe
DB_CONN = localhost:1521/xe

# Paramètres du compilateur
CXX = g++
CXXFLAGS = -Wall -Wextra -MMD -MP
LIBS = -locilib

# Répertoires
SRC_DIR = src
BIN_DIR = bin

# Fichiers source et fichiers binaires
SRCS = $(wildcard $(SRC_DIR)/*.cpp)
DEPS = $(SRCS:$(SRC_DIR)/%.cpp=$(BIN_DIR)/%.d)

# Créer le répertoire bin s'il n'existe pas
$(shell mkdir -p $(BIN_DIR))

.PHONY: clean import run_lecture run_menu setup

$(BIN_DIR)/lecture.out: $(SRC_DIR)/lectureDesDonnees.cpp
	$(CXX) $(CXXFLAGS) $< -o $@ $(LIBS)

$(BIN_DIR)/menu.out: $(SRC_DIR)/menu.cpp
	$(CXX) $(CXXFLAGS) $< -o $@ $(LIBS)

run_lecture: $(BIN_DIR)/lecture.out
	@./$< $(DB_USER) $(DB_PASSWORD)

run_menu: $(BIN_DIR)/menu.out
	@./$< $(DB_USER) $(DB_PASSWORD)

setup: import run_lecture
	@echo "Configuration terminée."

import:
	@echo exit | sqlplus -S $(DB_USER)/$(DB_PASSWORD)@$(DB_CONN) @./scripts/procedures.plsql

clean:
	rm -rf $(BIN_DIR)

-include $(DEPS)