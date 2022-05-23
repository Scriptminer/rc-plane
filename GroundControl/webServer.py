"""
Adapted from:
MIT license
(C) Konstantin Belyalov 2017-2018
"""
import gc
print("Imported GC", gc.mem_alloc() / (gc.mem_alloc()+gc.mem_free()) * 100)
import tinyweb
print("Imported Tinyweb", gc.mem_alloc() / (gc.mem_alloc()+gc.mem_free()) * 100)
import json
print("Imported JSON", gc.mem_alloc() / (gc.mem_alloc()+gc.mem_free()) * 100)
import time
print("Imported Time", gc.mem_alloc() / (gc.mem_alloc()+gc.mem_free()) * 100)

path = '/GroundControl'

from manageData import ManageData
manageData = ManageData(path+"/serverMessages.txt")
gc.collect()
print("Imported ManageData", gc.mem_alloc() / (gc.mem_alloc()+gc.mem_free()) * 100)
from serial import Serial
serial = Serial(64)
print("Imported Serial", gc.mem_alloc() / (gc.mem_alloc()+gc.mem_free()) * 100)

prevLoopTime = time.time()
avgLoopTime = 1

app = tinyweb.webserver() # Create web server application
print("Created tinyweb webserver", gc.mem_alloc() / (gc.mem_alloc()+gc.mem_free()) * 100)

# Index page
@app.route('/')
async def index(req, resp):
    # Just send file
    print("REQUEST CAME IN.")
    await resp.send_file(path+'/FlightStatusBetter.html')

@app.route('/static/<fn>')
async def staticFile(req, resp, fn):
    print("Serving {}".format(path+'/static/{}'.format(fn)))
    print("Mem usage: {0}%".format(round(gc.mem_alloc() / (gc.mem_alloc()+gc.mem_free()) * 100)))
    await resp.send_file(path+'/static/{}'.format(fn))
    gc.collect()

@app.route('/static/controls/<fn>')
async def staticImgFile(req, resp, fn):
    print("Serving {}".format(path+'/static/controls/{}'.format(fn)))
    print("Mem usage: {0}%".format(round(gc.mem_alloc() / (gc.mem_alloc()+gc.mem_free()) * 100)))
    resp.add_header("Content-Encoding","gzip")
    await resp.send_file(path+'/static/controls/{}'.format(fn))
    gc.collect()

@app.route('/api/data')
async def serveStatusData(req, resp):
    """ Serve the series of changing values, comma-separated, from the data table. """
    print("Serving status data to this client: ",resp)
    print("Mem usage: {0}%".format(round(gc.mem_alloc() / (gc.mem_alloc()+gc.mem_free()) * 100)))
    if gc.mem_free() < 2500: # Less than ~10% RAM left
        gc.collect()
        print("Aborting serving data!")
        await resp.send("[]")
        return

    outData_itterator = manageData.getStatusData() # Returns an itterator to itterate through all data
    await resp.send(b'[')
    for datapoint in outData_itterator:
        #print("Mem usage this loop: {0}%".format(round(gc.mem_alloc() / (gc.mem_alloc()+gc.mem_free()) * 100)))
        await resp.send(datapoint)
        await resp.send(b',')
        gc.collect()

    await resp.send("{}]") # Send closing bracket (with final JSON object to avoid issues with trailing comma on parse)

@app.route('/api/serverMessages')
async def serveServerMessages(req, resp):
    """ Serve the last 10 server messages, newline separated. """
    print("Serving server messages to: ",resp)
    outMessages = manageData.getServerMessages() # Serve the last 10 server messages
    for msg in outMessages:
        await resp.send(msg+"\n")

startTime = time.time()-1 # Temporary hack to prevent time difference and thus average time reaching 0

print("Defined some stuff", gc.mem_alloc() / (gc.mem_alloc()+gc.mem_free()) * 100)

def mainLoop():
    # Handles loop time
    global prevLoopTime
    global avgLoopTime
    timeDif = time.time() - prevLoopTime

    prevLoopTime = time.time()
    avgLoopTime = (timeDif+(avgLoopTime*3))/4

    currentTime = time.time() - startTime

    ramUsage = gc.mem_alloc() / (gc.mem_alloc()+gc.mem_free()) * 100

    manageData.handleDownData(serial.readData())
    manageData.handlePiLoopData(avgLoopTime,currentTime,ramUsage)
    print(".",end="")
    app.loop.create_task(mainLoop)

app.loop.create_task(mainLoop)
print("Created a task", gc.mem_alloc() / (gc.mem_alloc()+gc.mem_free()) * 100)
try:
    print("Beginning execution...")
    app.run(host='0.0.0.0', port=80)
except KeyboardInterrupt:
    print("Deleting temporary files...")
    open(path+"/data.json","w").close()
    print("Done.")
