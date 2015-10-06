# À tester avec chaque version
* Presets/sauvegarde de l'état. 
    * L'état de chaque paramêtre devrait être sauvegardé entre la fermeture et l'ouverture d'un projet. 
    * Pas besoin de tester dans chaque séquenceur, ni d'utiliser les presets pour tester ceci, car les même fonctions sont appellées dans chaque cas. 

* Contraintes de mouvement
    * S'assurer que chaque contrainte de mouvement fonctionne. Tester avec un nombre varié de sources. 
    * Les 3 contraintes "Equal" devraient prendre effet immédiatement.
        * Déplacer plusieurs sources différentes ne devrait pas "briser" le mode "equal". 
        * Déplacer les sources rapidements en passant par le milieu du dôme devrait introduide de petites erreurs de positions, mais ces erreurs devraient disparaître à la prochaine "action", ie, lorsqu'on modifie un autre paramêtre, sélectionne une autre source, etc.

* Onglet "Sliders"
    * S'assurer que chaque slider fonctionne correctement. Porter une attention particulière aux valeurs extrêmes. 

* Onglet Trajectories
    * Dans chaque séquenceur, tester chaque paramêtre possible de chaque trajectoire, en écriture (touch et write) et en lecture.
    * Aussi tester avec les différentes contraintes de mouvement

* Onglet Interfaces
    * Vérifier que les interfaces fonctionnent correctement. 
    * Pas vraiment besoin de tester dans différents séquenceurs. 
