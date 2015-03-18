scwev est un outil interactif permettant d'afficher les évènements issus par
l'USGS (United States Geologival Survey) par l'intermédiaire d'un flux RSS
(Rich Site Summary).


Onglet Map
==========

Onglet principal, il affiche les évènements en provenance du flux RSS et ceux
de la base de données sur la cartographie. Ces évènements sont aussi listés
dans une liste interactive repliable.

.. figure:: media/scwev/mainwindow.png
   :width: 20cm

   Onglet Map.


Onglet Events
=============

Il affiche la liste des évènements de l'inventaire local.

.. figure:: media/scwev/mainwindow_evtlist.png
   :width: 20cm

   Onglet Events.


Ces évènements peuvent être triés en fonction des choix de l'utilisateur. (Il
suffit de cliquer sur le chevron en bas à gauche pour afficher les filtres)

.. figure:: media/scwev/eventlist_filters.png
   :width: 20cm

   Filtres globaux de la liste des évènements locaux.

.. figure:: media/scwev/eventlist_filters1.png
   :width: 12cm

   Filtres du type d'évènements affichés. Ces derniers seront mis en surbrillance
   à l'aide d'une coloration particulière.


Les filtres sur les types d'évènement sont pré-configurable dans le fichier
de configuration de l'application.

.. code-block:: sh

   # Afficher les évènements 'Fake'
   wev.eventList.showNotExistingOrigins = false
   # Afficher les évènements non localisable
   wev.eventList.showNotLocatableOrigin = false
   # Afficher les évènements sans type
   wev.eventList.showOriginsWithNoType = false
   # Affcher les évènements en dehors de la zone d'intérêts
   wev.eventList.showOutOfNetworkInterestsOrigins = true


Flux RSS
========

Les données exploitées par scwev doivent être formatéés, pour être interprêtable,
au format `ATOM <http://www.w3.org/2005/Atom>`_.

.. code-block:: xml

   <feed xmlns="http://www.w3.org/2005/Atom" xmlns:georss="http://www.georss.org/georss">
      <updated>2014-04-14T13:02:57Z</updated>
      <title>USGS M 2.5+ Earthquakes</title>
      <subtitle>
         Real-time, worldwide earthquake list for the past day
      </subtitle>
      <link rel="self" href="http://earthquake.usgs.gov/earthquakes/catalogs/1day-M2.5.xml"/>
      <link href="http://earthquake.usgs.gov/earthquakes/"/>
      <author>
         <name>U.S. Geological Survey</name>
      </author>
      <id>http://earthquake.usgs.gov/</id>
      <icon>/favicon.ico</icon>
      <entry>
         <id>urn:earthquake-usgs-gov:announcement-deprecated</id>
         <title>Data Feed Deprecated</title>
         <updated>2014-04-14T13:02:57Z</updated>
         <link rel="alternate" type="text/html" href="http://earthquake.usgs.gov/earthquakes/catalogs/"/>
      </entry>
   </feed>

Bien que cette application fut testée avec l'utilisation des flux ATOM en 
provenance de l'`USGS <http://earthquake.usgs.gov/earthquakes/feed/v1.0/atom.php>`_
tout autre flux d'une autre agence empruntant ce format sera compatible et ses
données exploitable.

.. code-block:: sh

   # USGS live feed
   wev.rss.feed.USGS = "http://earthquake.usgs.gov/earthquakes/feed/v1.0/summary/"

.. note:: Les flux dont l'URL comporte *'usgs'* ne nécessitent que l'adresse
          racine du flux, l'extension du type de données sera générée à l'aide
          des boutons radio séléctionnés.
          
          Si l'URL du flux ne comporte pas *'usgs'*, il est impératif que cette
          URL soit complète.
          

Interactions
============

Chaque évènement de la cartographie est lié à un évènement de la liste locale
ou de la liste importée à l'aide du flux RSS.
Un double click sur l'entrée d'un évènement entraîne la création d'un marqueur
sur la cartographie indiquant l'évènement en question.

.. figure:: media/scwev/event_pin.png
   :width: 8cm
   
   Marqueur d'un évènement.

Un click sur un évènement de la cartographie entraîne la sélection de l'entrée
lui correspondant dans la liste des évènements du flux ou celle des évènements
de la base de données.

Les évènements de la liste locale sont dynamiquement effaçable de la cartographie
par l'utilisation des cases à cocher.

.. figure:: media/scwev/eventlist_evtselect.png
   :width: 18cm
   
   Sélection d'un évènement.

Dans le cas d'une utilisation de flux ATOM de l'USGS, l'utilisateur peut à
chaud changer le type d'évènements à afficher, ainsi que leur age limite.

.. figure:: media/scwev/usgs_choices.png
   :height: 8cm
   
   Sélection du type et de l'age limite des évènements.

scwev est suffisement autonome pour recycler, mettre à jour ses listes
d'évènements. L'utilisateur spécificie si cette option doit être active ou non.

.. code-block:: sh

   # Activer le raffraichissement auto
   wev.refresh.enable = true
   
   # Délai en secondes avant un raffraichissement
   wev.refresh.delay = 1800

Ces options sont altérable à chaud par l'intermédiaire du menu de configuration,
mais ces changements ne seront valable que pour la session active de l'application.

Il est possible d'importer les évènements issus du flux RSS dans la base de
données locale. Cette option n'est pas activée par défaut et doit l'être en 
utilisant le menu de configuration de l'application.

L'utilisateur peut aussi choisir de garder l'identifiant original du flux
lors de l'importation en base de données, autrement un nouvel identifiant
sera généré.

.. figure:: media/scwev/save_event.png
   :width: 18cm
   
   Le menu **Commit** permet d'importer l'évènement séléctionné.
   

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

Le système de cartographie utilisé par scwuv est basé sur une implémentation
en carreaux (tiles) normalisés (256x256 pixels). Il est possible d'utiliser
simulatanément plusieurs jeux de carreaux afin de commuter dynamiquement
les cartes.

.. code-block:: sh

   # Noms des jeux de carte
   map.names = "ESRI - Ocean Basemap", "Google - Hybrid Sat/Terrain",\
               "OpenStreetMap - Terrain"

   # Chemins des jeux de carte (dans l'ordre)
   map.paths = @DATADIR@/maps/esri/Ocean_Basemap/,\
               @DATADIR@/maps/gmap-hybrid/,\
               @DATADIR@/maps/opencyclemap/

Carreaux
--------

Les carreaux étant organisés en niveaux de zoom, colonnes et lignes, il est
possible de spécifier la manière dont ceux-ci sont archivés.

.. code-block:: sh

   # Pattern d'accès aux tiles
   # La pattern par défaut d'OpenStreetMap est "%1/%2/%3", ici on utilise
   # moins de sous répertoires.
   # map.tilePattern = "%1/osm_%1_%2_%3"
   # - %1 = niveau de zoom (0-*)
   # - %2 = colone (0- 2^zoom-1)
   # - %3 = ligne (0- 2^zoom-1 en projection Mercatique)
   # Chaque paramêtre peut être utilisé plus d'une fois.
   # @note Il n'est pas nécessaire de préciser l'extension des carreaux,
   #       l'algorithm a pour priorité les PNG, puis les JPG.
   map.tilePattern = "%1/%2/%3"

Plus d'informations sur l'organisation de carreaux 
`OpenStreetMap slippy map <http://wiki.openstreetmap.org/wiki/Slippy_map_tilenames>`_


Objets
------

Les objets sont régis par un système de feuilles de dessins (canvas) dans
lesquelles s'entremêlent des niveaux (layers) et des décorateurs. Par défaut,
les carreaux font parti du dessin en arrière plan, les objets (surnomés
'déssinables') font eux parti du dessin de premier plan.

Lors de l'affichage d'un évènement, les stations sont représentées par des
cercles pleins dont la couleur de remplissage varie en gradiant de bleu à rouge
en fonction des résidus des pointés (couleur = (255. / fabs(residuals)) % 255.).
Les épicentres sont eux représentés par des cercles creux dont la taille et la 
couleur varient respectivement en fonction de la magnitude de l'évènement
(taille = 4.9 * (magnitude - 1.2)) / 2)) et de la profondeur calculée.
Au dessus des objets, on retrouvera les décorateurs (graticules, autres
widgets, etc).



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
| Ctrl+I               | Affiche l'inventaire de base (stations)                     |
+----------------------+-------------------------------------------------------------+
| Ctrl+M               | Affiche l'onglet cartographie                               |
+----------------------+-------------------------------------------------------------+
| Ctrl+E               | Affiche l'onglet contenant la liste des évènements locaux   |
+----------------------+-------------------------------------------------------------+
| F3                   | Affiche la boîte de dialogue de configuration de scoiv      |
+----------------------+-------------------------------------------------------------+
| F11                  | Affiche l'application en plein écran                        |
+----------------------+-------------------------------------------------------------+
| Ctrl+Q               | Quitte l'application                                        |
+----------------------+-------------------------------------------------------------+


Execution de l'application
==========================

scwev ne requiert pas d'arguments particuliers lors de son exécution.

.. code-block:: sh

   seiscomp exec scwev
