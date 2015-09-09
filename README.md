# simple-upnpd
UPnP daemon which only announces the presence of the device

Well the title says it all. The only thing this program can do is announce its
presence on the network by UPnP. It will appear e.g. in the "Network Connections"
in Windows and redirected to a customizable url. That should be it, if it can do
more it is likely a BUG..

It is made to run on a linux machine, but likely runs anywhere where glib, gobject,
gupnp function properly..
