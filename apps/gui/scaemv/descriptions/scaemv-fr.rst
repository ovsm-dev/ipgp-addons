scaemv est un outil interactif regroupant une multitude de widgets:

- :ref:`Residual/Distance <fig-scaemv-residualdistance>`
- :ref:`Residual/Azimuth <fig-scaemv-residualazimuth>`
- :ref:`Particle Motion <fig-scaemv-particlemotion>`
- :ref:`Magnitude density <fig-scaemv-magnitudedensity>`
- :ref:`Drift to hypocenters <fig-scaemv-drifttohypocenters>`
- :ref:`Gutenberg Richter <fig-scaemv-gutenbergrichter>`
- :ref:`Cross section <fig-scaemv-crosssection>`
- :ref:`Magnitude variation <fig-scaemv-magnitudevariation>`
- :ref:`Uncertainties <fig-scaemv-uncertainties>`
- :ref:`Events map <fig-scaemv-eventsmap>`
- :ref:`Residual/Distance <fig-scaemv-residualdistance>`
- :ref:`Station stream <fig-scaemv-stationstream>`


Liste des évènements
====================

La liste des évènements affichée par scbev est configurable. Certaines colonnes
peuvent être affichées/masquées dynamiquement, et les évènements, origines
filtrés.

.. figure:: media/scbev/ctx_header.png
   :width: 14cm
   
   Menu contextuel des entêtes.

.. figure:: media/scbev/evnt_filters.png
   :width: 20cm
   
   Filtres divers.

Les requêtes auprès de la base de données pouvant être conséquentes, il est
recommandé d'utliser un thread secondaire, autre que celui de l'interface
graphique de l'application. Utiliser le cache temporaire aide aussi beaucoup
à réduire le nombre ces requêtes.

.. code-block:: sh

   # Should the GUI be using a separate thread to fetch objects ?
   # @note the interface will be responsive but this will input some delay on the
   #       entire operation
   bev.eventList.useSeparateThread = true
   
   # Should the GUI be using a cache to store objects
   # @note using a cache engine will reduce the time required to fetch objects
   #       but in the mean time, you should clear/refresh this cache periodically
   #       to ensure that objects are up to date  
   bev.eventList.useCacheEngine = true


L'utilisateur peut définir des filtres applicable aux localisateurs utilisés.

.. code-block:: sh

   # Noms des localisateurs
   bev.locators = LOCSAT, Hypo71, Earthworm, MCsefraN, NonLinLoc
   
   # Profiles du localisateur LOCSAT
   bev.locator.LOCSAT.profiles = iasp91, tab
   
   # Profiles du localisateur Hypo71
   bev.locator.Hypo71.profiles = "OVSM Dorel Volcanic", "OVSM Dorel Tectonic", "OVSM Landslide"

   # Profiles du localisateur Earthworm
   bev.locator.Earthworm.profiles = "EW Dorel profile"
   
   # Profiles du localisateur sefraN
   bev.locator.sefraN.profiles = TECT, VOLC
   
   # Profiles du localisateur NonLinLoc
   bev.locator.NonLinLoc.profiles = tectonique_ovsg, volcanique_ovsg, tectonique_ovsm,\
                                    volcanique_ovsm, LesSaintes_cdsa, tectonique_cdsa,\
                                    ak135_global, iasp91_global


L'utilisateur peut configurer le comportement par défaut du filtre pricipal
de la liste en fonction des types d'évènements visible.

.. code-block:: sh

   # Default origin type filters.
   # @note Those options set the application default event list composition.
   #       They are also configurable dynamically by using tool button but won't
   #       by saved when exiting.
   
   # Afficher/masquer les évènements non existant
   bev.showNotExistingOrigins = false
   
   # Afficher/masuer les évènements non localisable
   bev.showNotLocatableOrigin = false
   
   # Afficher/masquer les évènements dont le type n'as pas été défini
   bev.showOriginsWithNoType = false
   
   # Affcher/masquer les évènement en dehors des intérêts du réseau
   bev.showOutOfNetworkInterestsOrigins = true
   
   # Authoriser la suppression, création d'origines ou d'évènements
   aemv.eventList.allowAdvancedOptions = true

   # Mettre les évènements en surbrillance lorsqu'une interaction a lieu
   # à travers les autres widgets.
   # @e.g. les objets séléectionnés dans une coupe seront mis en surbrillance
   #       dans la liste
   aemv.eventList.decorateItems = true
   
   # Afficher des pins lorsqu'un évènement est sélectionné
   aemv.eventList.paintPinBox = true;
   
   # Afficher une ombre autour d'un pin d'un évènement séléctionné
   aemv.eventList.paintPinsShadow = false

   # Afficher les origines sans évènements
   aemv.eventList.fetchUnassociatedOrigins = false

   # Récupérer les commentaires des origines préférées
   aemv.eventList.fetchPreferredOriginComment = false
   
   # Récupérer les commentaires des origines non préférées
   aemv.eventList.fetchSiblingOriginComment = false # 'sibling' a.k.a. 'not preferred'
   
   # Récupérer les commentaires des origines sans évènements
   aemv.eventList.fetchUnassociatedOriginComment = false

.. note:: Ces options sont éditable en cours de session dans la boîte
          de dialogue de configuration.
             
          .. figure:: media/scbev/config_filters.png
             :width: 12cm
                
             Configuration des filters de types d'évènement.


.. _fig-scaemv-residualdistance:

Residual/Distance
=================

.. _fig-scaemv-residualazimuth:

Residual/Azimuth
================

.. _fig-scaemv-particlemotion:

Particle Motion
===============

scaemv dispose d'un visualiseur de mouvements de particles (ParticleMotion)
accessible dans le menu **File**, **Origine**, puis **ParticleMotion**. Ce
widget peut aussi être appelé par l'intermédiaire du bouton XYZ du summary
widget.

.. figure:: media/scwuv/pm_tabstations.png
   :width: 20cm

   Fenêtre ParticleMotion.

Chargement des données
----------------------

Les flux sont à l'instar de scolv, scrttv, etc rapatriés depuis les services
**record stream**. Il est possible d'utliser des filtres de base ainsi que
des filtres personnalisés.

.. code-block:: sh

   # URI du service habritant les données (seedlink, arclink)
   recordstream.uri = combined://seedsrv:18000;arcsrv:18001??rtMax=86400

   # Filtres de flux
   picker.filters = "BP 1 - 15 Hz 3rd order;BW(3,1,15)",\
                     "HP 1.5 3rd order;BW_HP(3,1.5)",\
                     "HP 3 n3;BW_HP(3,3)",\
                     "Eboul - BP 1 - 4 Hz n4;BW(4,1,4)",\
                     "BP 0.7 - 2 Hz;BW(3,0.7,2)",\
                     "4 pole HP @2s;BW_HP(4,0.5)",\
                     "4 pole LP @2s;BW_LP(4,0.5)"

.. warning:: Seules les données des stations disposant de pointé(s) sont chargées.

La première phase du chargement passe par la connection au service de paquets
miniseed, le délai d'attente maximal est de 30 secondes.

.. code-block:: sh

   # Délai maximal en seconde avant qu'une connexion soit invalidée par la
   # non réception de paquets (connection + data receive timeout)
   recordstream.timeout = 30

Les paquets miniseed sont reçus station par station. Le contrôle qualité est
exécuté en bloc.

.. figure:: media/scwuv/pm_qcdata.png
   :width: 20cm

   Attente de contrôle qualité.

Un fois l'intégrité des données contrôlée, l'utilisateur est informé de leur
état. Les graphiques dont les régions contiennent des trous (gap) ne pourront
êtré générés.

.. figure:: media/scwuv/pm_datagap.png
   :width: 20cm

   Contrôle qualité terminé.


Découpage d'une région
----------------------

L'utilisateur séléctionne une station à analyser par double click sur la ligne
correspondant à la station voulue.

Si la station sélectionnée dispose de pointés P et S, la section de données
incorporées par défaut et servant à la réalisation des graphiques est celle
dont les valeurs se trouvent dans l'intervalle de ces pointés.

.. figure:: media/scwuv/pm_defaultplot.png
   :width: 20cm

   Région entre pointés P et S.

Les drapeaux vert (début) et rouge (fin) de la barre d'outils
permettent d'affiner ou simplement redéfinir cette région.


Sélection d'un filtre
---------------------

L'utilisateur peut choisir d'appliquer un filtre sur les flux.

.. figure:: media/scwuv/pm_selectfilter.png
   :width: 12cm

   Application d'un filtre.


Changement de capteur
---------------------

Lors du chargement des données d'une station, le comportement par défaut est
d'afficher les données relatives à un capteur de type sismomètre. Si cette
station dispose de différents capteurs, il est possible d'en afficher les
données de différentes manières:

- utiliser la boîte **Stream**

  .. figure:: media/scwuv/pm_streamselectiona.png
     :width: 6cm

     Selection capteur, méthode a.

- utiliser le menu contextuel en cliquant sur les flux

  .. figure:: media/scwuv/pm_streamselectionb.png
     :width: 12cm

     Selection capteur, méthode b.


Export des données
------------------

Les données de la région selectionnée peuvent être sauvegardées.

.. figure:: media/scwuv/pm_savedata.png
   :width: 10cm

   Sauvegarde des données des graphes.

.. figure:: media/scwuv/pm_csvdata.png
   :width: 16cm

   Données au format CSV.


.. _fig-scaemv-magnitudedensity:

Magnitude density
=================

Ce widget affiche la concentration en magnitude des hypocentres en fonction des
latitudes, des longitudes et des profondeurs des évènements.

.. figure:: media/scaemv/mag_density.png
   :width: 14cm
   
   Widget magnitude density.


.. _fig-scaemv-drifttohypocenters:

Drift to hypocenters
====================

Ce widget affiche la dérive temporelle des hypocentres en fonction des latitudes,
des longitudes et des profondeurs des évènements.

.. figure:: media/scaemv/hypodrift.png
   :width: 18cm
   
   Widget hypocenter drift.


.. _fig-scaemv-gutenbergrichter:

Gutenberg Richter
=================

Ce widget affiche l'expression de la loi de Gutenberg Richter du catalogue
d'évènements en cours.

.. figure:: media/scaemv/grlaw.png
   :width: 20cm
   
   Widget Gutenberg-Richter.


.. _fig-scaemv-crosssection:

Cross section
=============

Couplé avec Events Map, ce widget affiche une coupe horizontale et verticale
des évènements situés dans une zone d'intérêt.

.. figure:: media/scaemv/cross_section.png
   :width: 24cm
   
   Widgets Events Map + Cross section.

Deux profiles de coupe sont nativement imlémentés: earthquake et volcano.

.. code-block:: sh
   
   # Latitude du point A
   aemv.crossSection.earthquake.latitude.a = 14.2
   
   # Longitude du point A
   aemv.crossSection.earthquake.longitude.a = -62.27
   
   # Latitude du point B
   aemv.crossSection.earthquake.latitude.b = 15.4
   
   # Longitude du point B
   aemv.crossSection.earthquake.longitude.b = -59.12
   
   # Largeur de la coupe en KM
   aemv.crossSection.earthquake.width = 80
   
   # Profondeur minimum des évènements
   aemv.crossSection.earthquake.depthMin = 3
   
   # Profondeur maximale des évènements
   aemv.crossSection.earthquake.depthMax = 200
   
   # Même procédure pour le profile volcano...
   aemv.crossSection.volcano.latitude.a = 14.807
   ...


.. _fig-scaemv-magnitudevariation:

Magnitude variation
===================

Ce widget affiche les variations de valeurs de magnitude cummulées 
quotidiennement ou mensuellement.

.. figure:: media/scaemv/mv1.png
   :width: 24cm
   
   Magnitudes cummulées en barres.
   
.. figure:: media/scaemv/mv2.png
   :width: 24cm
   
   Magnitudes cummulées en courbes pleines empilées.
   
.. figure:: media/scaemv/mv3.png
   :width: 24cm
   
   Courbes de magnitudes.


.. _fig-scaemv-uncertainties:

Uncertainties
=============

Ce widget affiche des graphiques relatifs aux incertitudes cummulées et
regroupées par latitudes, longitudes, profondeurs, R.M.S., phases et
magnitudes.

.. figure:: media/scaemv/uncertainties.png
   :width: 20cm
   
   Widget uncertainties.



.. _fig-scaemv-eventsmap:

Events Map
==========

Ce widget affiche une carte de la sismicité des éléments de la liste des
évènements. Il reprend les principales propriétés de cartographie en plus
d'être interactif avec les autres widgets.

Options d'affichage:

.. code-block:: sh

   # Afficher des pins pour signaler un point d'intérêt (POI) sur la carte
   # lorsqu'un object reçoit un click
   aemv.eventMap.paintPins = true
   
   # Afficher une boîte d'info accompagnant un pin   
   aemv.eventMap.paintPinsWithBox = true
   
   # @note Cette option est activée pour accentuer l'effet des pins sur la carte
   #       mais s'accompagne de ralentissement du processus de redessinage de
   #       la carte sur les systèmes légers (faible CPU)
   aemv.eventMap.paintPinsWithShadow = false
   
   # Effacer les pins automatiquement
   aemv.eventMap.clearPinsAuto = true
   
   # Delay d'effacement des pins
   aemv.eventMap.clearPinsDelay = 5


.. _fig-scaemv-stationstream:

Station stream
==============

Ce widget permet d'afficher l'ensemble des flux actifs en temps réel des
stations de l'inventaire.

.. figure:: media/scaemv/stream.png
   :width: 20cm
   
   Widget station stream.



Interactions entre widgets
==========================

Les widgets puisent leurs données à travers la liste principale. Ils évoluent
donc au gré de cette dernière : lorsqu'un changement intervient, ils ont
généralement la capacité de se mettre à jour, autrement, l'utlisateur est
visuellement alerté par ces derniers afin qu'il puissent intervenir.



Raccourcis clavier
==================

Le tableau suivant décrit les raccourcis disponible et leurs actions dans scamev.

+----------------------+-------------------------------------------------------------+
| Raccourci            | Description                                                 |
+======================+=============================================================+
| F1                   | Affiche l'index de l'aide en ligne                          |
+----------------------+-------------------------------------------------------------+
| Shift+F1             | Affiche cette page de l'aide en ligne                       |
+----------------------+-------------------------------------------------------------+
| F2                   | Affiche la boîte de dialogue de configuration de            |
|                      | connections (bdd  / master)                                 |
+----------------------+-------------------------------------------------------------+
| F3                   | Affiche la boîte de dialogue de configuration de scaemv     |
+----------------------+-------------------------------------------------------------+
| F8                   | Affiche le widget Summary                                   |
+----------------------+-------------------------------------------------------------+
| F9                   | Affiche le log de scaemv                                    |
+----------------------+-------------------------------------------------------------+
| Ctrl+Shift+D         | Affiche le widget Residual/Distance                         |
+----------------------+-------------------------------------------------------------+
| Ctrl+Shift+A         | Affiche le widget Residual/Azimuth                          |
+----------------------+-------------------------------------------------------------+
| Ctrl+Shift+H         | Affiche le widget Drift to hypocenters                      |
+----------------------+-------------------------------------------------------------+
| Ctrl+Shift+G         | Affiche le widget Gutenberg-Richter                         |
+----------------------+-------------------------------------------------------------+
| Ctrl+Shift+T         | Affiche le widget Cross section                             |
+----------------------+-------------------------------------------------------------+
| Ctrl+Shift+V         | Affiche le widget Magnitude variation                       |
+----------------------+-------------------------------------------------------------+
| Ctrl+Shift+U         | Affiche le widget Uncertainties                             |
+----------------------+-------------------------------------------------------------+
| Ctrl+Shift+M         | Affiche le widget Events Map                                |
+----------------------+-------------------------------------------------------------+
| Ctrl+Shift+L         | Affiche le widget Station stream                            |
+----------------------+-------------------------------------------------------------+


Unités et décimales
===================

Les préférences des utlisateurs en matière d'unités de distance variant d'un
utlisateur à un autre, scwuv peut être configuré pour afficher ces informations
de façon personnalisée.

.. code-block:: sh

   # If true, show distances in km. Use degree otherwise.
   scheme.unit.distanceInKM = true

Il en est de même pour le nombre de virgules flottantes:

.. code-block:: sh
   
   # Précision par défault
   scheme.precision.general = 2
   
   # Precision of depth values.
   scheme.precision.depth = 0
   
   # Precision of lat/lon values.
   scheme.precision.location = 2
   
   # Distances
   scheme.precision.distance = 2
   
   # R.M.S
   scheme.precision.rms = 1
   
   # Azimuth
   scheme.precision.azimuth = 0
   
   # Precision of pick times (fractions of seconds).
   scheme.precision.pickTime = 1


Cartographie
============

Le système de cartographie utilisé par scaemv est basé sur une implémentation
en carreaux (tiles) normalisés (256x256 pixels). Il est possible d'utiliser
simulatanément plusieurs jeux de carreaux afin de commuter dynamiquement
les cartes.

.. code-block:: sh

   # Noms des jeux de carte
   map.names = "ESRI - Ocean Basemap", "Google - Hybrid Sat/Terrain",\
               "OpenStreetMap - Terrain"

   # Chemins des jeux de carte (dans l'ordre)
   map.paths = ${HOME}/.ipgp/maps/esri/Ocean_Basemap/,\
               ${HOME}/.ipgp/maps/gmap-hybrid/,\
               ${HOME}/.ipgp/maps/opencyclemap/

Carreaux
--------

Les carreaux étant organisés en niveaux de zoom, colonnes et lignes, il est
possible de spécifier la manière dont ceux-ci ont été archivés et doivent
être lu.

.. code-block:: sh

   # Pattern d'accès aux tiles
   # La pattern par défaut d'OpenStreetMap est "%1/%2/%3".
   # Dans notre configuration les tiles sont regroupées en un niveau de zoom
   # par répertoire. On a donc moins de sous répertoires.
   # - %1 = niveau de zoom (0-*)
   # - %2 = colone (0- 2^zoom-1)
   # - %3 = ligne (0- 2^zoom-1 en projection Mercatique)
   # Chaque paramêtre peut être utilisé plus d'une fois.
   # @note Il n'est pas nécessaire de préciser l'extension des carreaux,
   #       l'algorithm a pour priorité les PNG, puis les JPG.
   map.tilePattern = "%1/osm_%1_%2_%3"

Plus d'informations sur l'organisation de carreaux 
`OpenStreetMap slippy map <http://wiki.openstreetmap.org/wiki/Slippy_map_tilenames>`_


Objets
------

Les objets sont régis par un système de feuilles de dessins (canvas) dans
lesquelles s'entremêlent des niveaux (layers) et des décorateurs.
Par défaut, les carreaux font parti du dessin en arrière plan, les objets (surnomés
'déssinables') font eux parti du dessin de premier plan.

Lors de l'affichage d'un évènement, les stations sont représentées par des
cercles pleins dont la couleur de remplissage varie en gradiant de bleu à rouge
en fonction des résidus des pointés (couleur = (255. / fabs(residuals)) % 255.).
Les épicentres sont eux représentés par des cercles pleins dont la taille et la 
couleur varient respectivement en fonction de la magnitude de l'évènement
(taille = 4.9 * (magnitude - 1.2)) / 2)) et de la profondeur calculée. Il est
possible de changer ce paramêtre et laisser l'algorithme utiliser un ratio
différent: la carte générée sera plus proche d'une carte de macrosismicité.
Au dessus des objets, on retrouvera les décorateurs (graticules, autres
widgets, etc).



Execution de l'application
==========================

scaemv ne requiert pas d'arguments particuliers lors de son exécution.

.. code-block:: sh

   seiscomp exec scaemv
   


 