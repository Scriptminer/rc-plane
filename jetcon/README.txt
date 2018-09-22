Install the dependencies
------------------------

pip install tornado

pip install sockjs

git clone https://github.com/mrjoes/sockjs-tornado.git
cd sockjs-tornado
python setup.py build
python setup.py install

Run the code
------------

python jetcon.py

and point your browser at http://<IPADDR>:8080/ where you can
work out <IPADDR> by looking at the output of ifconfig.

Credits
-------

Thanks to sockjs-tornado for example code.
Thanks to http://projecthabu.com/post/91905257923/this-sr-71-blackbird-cockpit-got-more-flight
for the Blackbird image.
