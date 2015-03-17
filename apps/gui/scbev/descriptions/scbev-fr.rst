scbev est un outil interactif permettant l'export de catalogues et de bulletins
d'hypocentres en divers formats:

- :ref:`HYPOINVERSE <fig-scbev-hypoinverse>`
- :ref:`GSE 2.0 <fig-scbev-gse2.0>`
- :ref:`IMS 1.0 <fig-scbev-ims1.0>`
- :ref:`QuakeML <fig-scbev-quakeml>`


.. figure:: media/scbev/mainwindow.png
   :width: 16cm

   Interface scbev.


Liste des évènements
====================

La liste des évènements affichée par scbev est configurable. Certaines colonnes
peuvent être affichées/masquées dynamiquement, et les évènements, origines
filtrés.

*Les codes séisme (colonne Scode) ne sont pas directement issus d'un champ
dédié ou d'une table de la base de données des évènements, mais proviennent
d'un commentaire, pour chaque évènement, préhalablement sauvegardé lors de
la synchronisation de ce dernier vers le WebObs. Les évènements non synchronisés
ne disposent pas de code séisme.*


.. figure:: media/scbev/ctx_header.png
   :width: 10cm
   
   Menu contextuel des entêtes.

.. figure:: media/scbev/evnt_filters.png
   :width: 16cm
   
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

.. note:: Ces options sont éditable en cours de session dans la boîte
          de dialogue de configuration.
             
          .. figure:: media/scbev/config_filters.png
             :width: 12cm
                
             Configuration des filters de types d'évènement.


.. _fig-scbev-hypoinverse:

HYPOINVERSE
===========

Format historique d'Hypo71, un programme fortran pour résoudre les positions
et magnitudes des séismes, HYPOINVERSE (HYPO2000) a été développé à l'USGS par
Fred Klein.

Il se compose:

* d'un catalogue contenant la liste des sommaires des hypocentres

  .. code-block:: sh

     20140407 0603 33.68 14-00.94  60-21.05  36.62 D 2.68 24 286 67.3 0.08  1.0  5.0 C  TE1   20140407_060300_a.mq0
     20140407 0455 52.04 15-47.23  60-27.69  16.61 D 2.88 18 244 87.0 0.13  1.2  2.7 C  TE1   20140407_045500_a.mq0
     20140406 1225 48.75 13-27.37  59-04.05  54.30 D 3.59 15 343215.5 0.17 13.4110.6 D  TE1   20140406_122500_a.mq0
     20140405 2049 12.75 14-49.03  61-12.72   1.00 D 1.93  4 319  0.3 0.10           C  EB1   20140405_204900_a.mq0
     20140405 1746 46.68 14-36.37  61-00.73   8.33 D 1.50  7 260  2.6 0.03  0.4  0.3 C  TE1   20140405_174600_a.mq0
     20140405 1709 16.58 14-34.51  61-00.01   7.11 D 1.16 11 109  2.7 0.11  0.4  0.6 B  TE1   20140405_170900_a.mq0
     20140405 1126 42.39 13-50.50  60-43.63  30.10 D 2.08 12 250 27.4 0.13  2.7  5.6 D  TE1   20140405_112600_a.mq0
     20140405 0359 16.67 14-48.87  61-09.86  -1.00 D 1.78  6 175  0.3 0.11  0.4  0.6 B  EB1   20140405_035900_a.mq0
     20140404 1640 24.98 14-46.92  60-35.01  90.11 D 2.29 12 305 45.9 0.13 10.7  9.2 D  TE1   20140404_164000_a.mq0
     ...

* de bulletins de phases associés à la liste des sommaires des hypocentres

  .. code-block:: sh

     FDF EP 2 14040706 351.62       65.20ES 3                                      M=2.7 TE1   20140407_060300_a.mq0
     MLM EP 2 14040706 352.53                                                      
     GBM EP 0 14040706 352.47                                                      
     LAM EP 2 14040706 352.78                                                      
     BAM EP 2 14040706 352.36                                                      
     IA2 EP 2 14040706 352.96       67.37ES 2                                      
     ZAM EP 0 14040706 348.88       60.32ES 2                                      
     TRM EP 2 14040706 348.61       60.16ES 2                                      
     LPM EP 1 14040706 348.28       59.27ES 1                                      
     BIM EP 2 14040706 348.96       60.26ES 3                                      
     ...

La jointure entre ces deux éléments est réalisée à l'aide d'un dénominateur
commun que l'utlisateur peut spécifier. Ce dernier a le choix entre:

* l'eventID de l'évènement
* l'originID de d'évènement
* ou encore une expression propre

.. code-block:: sh

   # HYPOINVERSE LINKER
   # @note The user may choose between:
   #         - "%eventID%"
   #         - "%originID%"
   #         - custom patternID -> bev.%Y%m%d_%H%M%S.%f_mq0
   bev.export.instituteTag = %Y%m%d_%H%M00_a.mq0



.. _fig-scbev-gse2.0:

GSE 2.0
=======

Développé pour le Group of Scientiﬁc Experts et testé, expérimenté  au sein de
la FDSN (Federation of Digital Seismograph Networks), ce format a démontré ses
capacités pour l'échange international d'informations.
Structuré pour être échangé principalement par emails, il est basé sur
l'AutoDRM, un système automatisé développé pour fournir des informations sur les
données, stations et évènements d'un réseau local en réponse à des messages
de requête.

Message d'exemple:

.. code-block:: sh

   BEGIN GSE2.0
   MSG_TYPE DATA
   MSG_ID 2014/04/08_124357 REV_OVSM
   DATA_TYPE BULLETIN GSE2.0
   EVENT ovsm2014gtwh Windward Islands
       Date      Time       Latitude Longitude    Depth    Ndef Nsta Gap    Mag1  N    Mag2  N    Mag3  N  Author          ID
          rms   OT_Error      Smajor Sminor Az        Err   mdist  Mdist     Err        Err        Err     Quality

   2014/04/07 06:03:33.6    14.0157  -60.3510      36.6      25   24 286             ML 2.8  9  Md 2.7 11  Bulletin  L1l0HErO
         0.08   +-  0.00                   286    +-  5.0    0.61   1.15   +-         +-0.2      +-0.2     m i ke

   Sta     Dist  EvAz     Phase      Date       Time     TRes  Azim  AzRes  Slow  SRes Def   SNR       Amp   Per   Mag1   Mag2       ID
   FDF     1.05 313.0 m i P       2014/04/07 06:03:51.6  -0.0  95.0                          2.0       0.3       ML 2.6        7dx0atQd
   FDF     1.05 313.0 m i S       2014/04/07 06:04:05.2  -0.1  95.0                                                            Q7CoA40c
   MLM     1.11 313.0 m i P       2014/04/07 06:03:52.5   0.1  95.0                          5.4       0.8       ML 3.0        pE6iTdjs
   GBM     1.11 315.0 m i P       2014/04/07 06:03:52.4   0.0  95.0                          6.3       0.8       ML 3.0        5sgAXyeS
   LAM     1.12 315.0 m i P       2014/04/07 06:03:52.7   0.1  95.0                                    1.4       ML 3.3        zBHxTnAz
   BAM     1.11 316.0 m i P       2014/04/07 06:03:52.3  -0.1  95.0                          4.7       0.8       ML 3.0        i2LGVCGa
   IA2     1.14 313.0 m i P       2014/04/07 06:03:52.9   0.2  94.0                                    0.6       ML 2.9        4DiLGp1z
   IA2     1.14 313.0 m i S       2014/04/07 06:04:07.3   0.1  94.0                                                            H7Yst29Q
   SAM     1.14 316.0 m i S       2014/04/07 06:04:07.4   0.0  94.0                                                            sGdbRByZ
   ZAM     0.86 311.0 m i P       2014/04/07 06:03:48.8        97.0                                    1.0       ML 3.0        savdA2bu
   ZAM     0.86 311.0 m i S       2014/04/07 06:04:00.3  -0.1  97.0                                                            th5XuWlZ
   TRM     0.85 307.0 m i P       2014/04/07 06:03:48.6  -0.2  97.0                                  180.7 31.52 Md 2.5        R9ib1fPj
   LPM     0.82 314.0 m i P       2014/04/07 06:03:48.2  -0.0  98.0                                    0.5       ML 2.7        LTisiHfA
   LPM     0.82 314.0 m i S       2014/04/07 06:03:59.2  -0.1  98.0                                                            FAB5qCpH
   BIM     0.86 306.0 m i P       2014/04/07 06:03:48.9   0.1  97.0                                    0.6       ML 2.8        Hkv14ERL
   BIM     0.86 306.0 m i S       2014/04/07 06:04:00.2  -0.1  97.0                                                            DXKs2qCB
   MPOM    0.65 311.0 m i P       2014/04/07 06:03:46.1   0.1 101.0                          1.4       0.4       ML 2.5        7IRffUGK
   MPOM    0.65 311.0 m i S       2014/04/07 06:03:55.3  -0.0 101.0                                                            i7FzyU43
   SLBI    0.61 271.0 m i P       2014/04/07 06:03:45.3  -0.0 103.0                          1.4       0.4       ML 2.5        2pVURAwk
   SLBI    0.61 271.0 m i S       2014/04/07 06:03:54.2   0.1 103.0                                                            l6Ddk3Ch
   MCLT    0.65 242.0 m i P       2014/04/07 06:03:45.9  -0.1 101.0                          1.5       0.4       ML 2.4        BhlH1Rf7
   TRM     0.85 307.0 m i S       2014/04/07 06:04:00.1  -0.1  97.0                                                            MLw8tp2k
   SAM     1.14 316.0 m i P       2014/04/07 06:03:52.7  -0.1  94.0                                    2.3       ML 3.5        CN4kboum
   CPM     1.15 314.0 m i P       2014/04/07 06:03:53.1   0.2  94.0                          2.8       0.4       ML 2.7        jVntOPzg

   ...

   STOP


.. _fig-scbev-ims1.0:

IMS 1.0
=======

International Monitoring System 1.0 (IMS1.0) est un type de message BULLETIN
IASPEI Seismic Format (ISF) régit par IRIS DMC.
Très proche du format GSE, il est concidéré comme une amélioration de ce
dernier, seules quelques sections évoluent.

Message d'exemple:

.. code-block:: sh

   BEGIN IMS1.0
   MSG_TYPE DATA
   MSG_ID 2014/04/08_124906 REV_OVSM
   BULLETIN (IMS1.0:SHORT FORMAT)
   DATA_TYPE BULLETIN IMS1.0:short
   EVENT ovsm2014gtwh Windward Islands
      Date       Time        Err   RMS Latitude Longitude  Smaj  Smin  Az Depth   Err Ndef Nsta Gap  mdist  Mdist Qual   Author      OrigID
   2014/04/07 06:03:33.68   0.00  0.08  14.0157  -60.3510   0.0   0.0   0  36.6   5.0   25   24 286   0.61   1.15 g ke Bulletin  za3eeVqt

   Magnitude  Err Nsta Author      OrigID
   MLv    2.7 0.2   13 Bulletin  za3eeVqt
   ML     2.8 0.2    9 Bulletin  za3eeVqt
   Md     2.7 0.2   11 Bulletin  za3eeVqt
   Sta     Dist  EvAz Phase        Time      TRes  Azim AzRes   Slow   SRes Def   SNR       Amp   Per Qual Magnitude    ArrID
   FDF     1.05 313.0 P        06:03:51.621 -0.0   95.0                     ___   2.0       0.3       m__ MLv    2.6 u2l6ZoGd
   FDF     1.05 313.0 S        06:04:05.200 -0.1   95.0                     ___                       m__        0.0 Rvxnj0ft
   MLM     1.11 313.0 P        06:03:52.532  0.1   95.0                     ___   5.4       0.8       m__ MLv    3.0 xeqlZuup
   GBM     1.11 315.0 P        06:03:52.470  0.0   95.0                     ___   6.3       0.8       m__ MLv    3.0 sntx7JOr
   LAM     1.12 315.0 P        06:03:52.777  0.1   95.0                     ___             1.4       m__ ML     3.3 A0lpcSqR
   BAM     1.11 316.0 P        06:03:52.358 -0.1   95.0                     ___   4.7       0.8       m__ MLv    3.0 dM5lC8CZ
   IA2     1.14 313.0 P        06:03:52.961  0.2   94.0                     ___             0.6       m__ ML     2.9 bS90brdr
   IA2     1.14 313.0 S        06:04:07.370  0.1   94.0                     ___                       m__        0.0 FxOC4rRs
   SAM     1.14 316.0 S        06:04:07.421  0.0   94.0                     ___                       m__        0.0 h2HhKYXM
   ZAM     0.86 311.0 P        06:03:48.878        97.0                     ___             1.0       m__ ML     3.0 AQX0OpOP
   ZAM     0.86 311.0 S        06:04:00.319 -0.1   97.0                     ___                       m__        0.0 5NE64Hmx
   TRM     0.85 307.0 P        06:03:48.612 -0.2   97.0                     ___           180.7 31.52 m__ Md     2.5 2YZXfEdk
   LPM     0.82 314.0 P        06:03:48.275 -0.0   98.0                     ___             0.5       m__ ML     2.7 xLs5xdHV
   LPM     0.82 314.0 S        06:03:59.265 -0.1   98.0                     ___                       m__        0.0 TuLvHpbD
   BIM     0.86 306.0 P        06:03:48.960  0.1   97.0                     ___             0.6       m__ ML     2.8 0DxTaJg1
   BIM     0.86 306.0 S        06:04:00.258 -0.1   97.0                     ___                       m__        0.0 y4MDyPMV
   MPOM    0.65 311.0 P        06:03:46.146  0.1  101.0                     ___   1.4       0.4       m__ MLv    2.5 q2RN5mwM
   MPOM    0.65 311.0 S        06:03:55.336 -0.0  101.0                     ___                       m__        0.0 F87cn6Fe
   SLBI    0.61 271.0 P        06:03:45.327 -0.0  103.0                     ___   1.4       0.4       m__ MLv    2.5 z1Xyy1pW
   SLBI    0.61 271.0 S        06:03:54.241  0.1  103.0                     ___                       m__        0.0 V1oiG03V
   MCLT    0.65 242.0 P        06:03:45.911 -0.1  101.0                     ___   1.5       0.4       m__ MLv    2.4 SIwLTTno
   TRM     0.85 307.0 S        06:04:00.155 -0.1   97.0                     ___                       m__        0.0 Rkq2hU4E
   SAM     1.14 316.0 P        06:03:52.726 -0.1   94.0                     ___             2.3       m__ ML     3.5 LP29HfUq
   CPM     1.15 314.0 P        06:03:53.115  0.2   94.0                     ___   2.8       0.4       m__ MLv    2.7 66yBUqmA

   ...
   
   STOP

Pour plus d'informations `Documentation PDF` `<http://www.isc.ac.uk/standards/isf/download/ims1_0.pdf>`_


.. _fig-scbev-quakeml:

QuakeML
=======

QuakeML est un standard ouvert développé pour être distribué de façon
transparente. Il est flexible, extensible et modulaire à l'aide de sa
représentation XML des données sismologiques. Il a été conçu pour couvrir
un maximum d'applications possible à l'heure de la sismologie actuelle.

Sa structure étant le XML, il se compose d'un ensemble de noeuds dans lequels
s'articulent les informations.

Message d'exemple:

.. code-block:: xml

   <q:quakeml xmlns:q="http://quakeml.org/xmlns/quakeml-rt/1.2" xmlns="http://quakeml.org/xmlns/bed-rt/1.2">
       <eventParameters publicID="smi:scs/0.7/NA-N0x1b309e0N0x1bdab00">
           <stationMagnitude publicID="smi:scs/0.7/StationMagnitude#20140219155602.884486.8169">
               <comment/>
                <originID>smi:scs/0.7/NLL.20140219155533.118353.8091</originID>
                <mag>
                    <value>6.477554536</value>
                </mag>
                <type>MLv</type>
                <amplitudeID>smi:scs/0.7/Amplitude#20140218132117.182366.2946</amplitudeID>
                <waveformID networkCode="CU" stationCode="ANWB" locationCode="00" channelCode="BHZ">smi:scs/0.7/NA-N0x1b309e0N0x1c02b28</waveformID>
                <creationInfo>
                    <agencyID>OVSM</agencyID>
                    <author>tartampion</author>
                    <creationTime>2014-02-19T15:56:02.884506Z</creationTime>
                </creationInfo>
            </stationMagnitude>
        ...
            <origin publicID="smi:scs/0.7/NLL.20140219155533.118353.8091">
                <time>
                    <value>2014-02-18T09:27:13.923195Z</value>
                </time>
                <latitude>
                    <value>14.71801758</value>
                    <uncertainty>2.421217758</uncertainty>
                </latitude>
                <longitude>
                    <value>-59.07348633</value>
                    <uncertainty>3.956615503</uncertainty>
                </longitude>
                <depth>
                    <value>10.66227214</value>
                    <uncertainty>6.500011675</uncertainty>
                </depth>
                <arrival publicID="smi:scs/0.7/NA-N0x1b309e0N0x1c41640">
                    <comment/>
                    <pickID>smi:scs/0.7/Pick#20140219145921.479921.7757</pickID>
                    <phase>S</phase>
                    <timeCorrection>0</timeCorrection>
                    <azimuth>145.9949597</azimuth>
                    <distance>11.50923644</distance>
                    <timeResidual>-18.5235464</timeResidual>
                    <timeWeight>0</timeWeight>
                </arrival>
            </origin>
       </eventParameters>
   </q:quakeml>


Pour plus d'informations `QuakeML` `<https://quake.ethz.ch/quakeml/>`_


La génération de bulletins au format QuakeML requiert un schéma, ce dernier
est utilisé lors de la conversion des données initialement au format SC3ML.

.. warning:: La version de ce schéma doit correspondre avec la version de la
             base de données utilisée par le master de SeisComP3 afin que tous
             les objets soient reconnus et puissent être traités correctement
             lors de la conversion.

.. code-block:: sh

   # Schéma QuakeML
   bev.export.quakeml.schemaFile = @DATADIR@/scbev/sc3ml_0.7__quakeml_1.2.xsl


Aussi, cette conversion est éffectuée à l'aide de l'application **xalan**
disponible dans les paquets de distribution Linux.

.. code-block:: sh

   # Sur Debian
   apt-get install xalan
   
   # Sur Red Hat
   yum install xalan


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
| F3                   | Affiche la boîte de dialogue de configuration de scoiv      |
+----------------------+-------------------------------------------------------------+
| F8                   | Affiche le widget Summary                                   |
+----------------------+-------------------------------------------------------------+
| Ctrl+P               | Ouvre la boîte de dialogue d'export de catalogue+bulletin   |
+----------------------+-------------------------------------------------------------+
| Ctrl+Q               | Quitte l'application                                        |
+----------------------+-------------------------------------------------------------+
| Ctrl+M               | Ouvre la boîte de dialogue d'export de la carte de          |
|                      | sismicité                                                   |
+----------------------+-------------------------------------------------------------+
| Ctrl+H               | Affiche les crédit de l'application                         |
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


Execution de l'application
==========================

scbev ne requiert pas d'arguments particuliers lors de son exécution.

.. code-block:: sh

   seiscomp exec scbev
