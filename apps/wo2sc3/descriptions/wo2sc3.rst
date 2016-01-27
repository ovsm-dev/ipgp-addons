Receives, parses, analyzes and extracts data in order to create and send
origin objects to the messaging system, remove events from the database.


Communication
=============

wo2sc3 awaits for connections from clients thru a specified socket port. When
an authorized host connects, wo2sc3 listens for messages.


Processing
==========

wo2sc3 reads XML data streams (messages) transmitted by clients. It tries and
parses them in order to extract valuable information.
To be valid, incoming messages shall match configured moduleID and typeID,
otherwise they will be properly rejected by wo2sc3.


**Origin creation message:**

.. code-block:: xml

   <?xml version="1.0" encoding="utf-8"?>
   <!DOCTYPE WO2SC3 SYSTEM "wo2sc3.dtd">
   <webObs>
      <moduleDescription>
        <id>1</id>
        <type>1</type>
      </moduleDescription>
      <eventDescription>
        <date>2014/01/07</date>
        <time>18:10:15</time>
        <station>ILAM</station>
        <network>WI</network>
        <duration>13</duration>
        <sminusp>20</sminusp>
        <operator>SR</operator>
        <mcid>MC3.OVSM.20131218</mcid>
        <type>TELE</type>
        <comment>This is a comment, get used to it!</comment>
      </eventDescription>
    </webObs>

This message will translate into an SeisComP3 origin

=========================================================  =====================================================
Seiscomp::DataModel::Origin                                XML message
=========================================================  =====================================================
Origin.time.value                                          date + time
Origin.author                                              operator
Origin.comment                                             fileID + sminusp + mcid + type + comment
Origin.latitude.uncertainty                                _
Origin.longitude.uncertainty                               _
Origin.depth.uncertainty                                   _
Origin.originQuality.standardError                         _
Origin.originQuality.secondaryAzimuthalGap                 _
Origin.originQuality.usedStationCount                      1
Origin.originQuality.associatedStationCount                1
Origin.originQuality.associatedPhaseCount                  associatedPhaseCount
Origin.originQuality.usedPhaseCount                        associatedPhaseCount
Origin.originQuality.depthPhaseCount                       _
Origin.originQuality.minimumDistance                       _
Origin.originQuality.maximumDistance                       _
Origin.originQuality.medianDistance                        _
Origin.originQuality.groundTruthLevel                      _
Origin.originUncertainty.horizontalUncertainty             _
Origin.originUncertainty.minHorizontalUncertainty          _
Origin.originUncertainty.maxHorizontalUncertainty          _
Origin.originUncertainty.azimuthMaxHorizontalUncertainty   _
ConfidenceEllipsoid.semiMajorAxisLength                    _
ConfidenceEllipsoid.semiMinorAxisLength                    _
ConfidenceEllipsoid.semiIntermediateAxisLength             _
ConfidenceEllipsoid.majorAxisPlunge                        _
ConfidenceEllipsoid.majorAxisAzimuth                       _
ConfidenceEllipsoid.majorAxisRotation                      _
=========================================================  =====================================================

Objects are not saved into the database directly by wo2sc3 but are sent to the
master (scmaster) so they can be dealt with by scevent. The latter should be
configured properly to accept origins that have a minimum phases number to 1.


**Event removal message:**

.. code-block:: xml

   <?xml version="1.0" encoding="utf-8"?>
   <!DOCTYPE WO2SC3 SYSTEM "wo2sc3.dtd">
   <webObs>
     <moduleDescription>
       <type>1</type>
       <id>1</id>
     </moduleDescription>
     <objectRemoval>
       <eventID>ovsm2012zxyd</eventID>
     </objectRemoval>
   </webObs>

wo2sc3 tries and fetches the concerned event. If found, it processes to its
removal by using the user specified method:

- *hide* will set the event's type to **not existing**

- *delete* will completely erase the event and all its children (origins,
  arrivals, picks, moment tensors, etc).

 