"""
Adapted from:
MIT license
(C) Konstantin Belyalov 2017-2018
"""
import gc
import tinyweb
import time

def get_mem_usage():
    return "Mem usage: {0}%".format(round(gc.mem_alloc() / (gc.mem_alloc()+gc.mem_free()) * 100))

print("Imported GC, tinyweb, time.", get_mem_usage())

path = '/GroundControl'
gc.collect()
print(gc.mem_free())
from manageData import ManageData
manageData = ManageData(path+"/serverMessages.txt")
gc.collect()
print("Imported ManageData", get_mem_usage())
from serial import Serial
serial = Serial(64)
print("Imported Serial", get_mem_usage())

app = tinyweb.webserver() # Create web server application
print("Created tinyweb webserver", get_mem_usage())

# Index page
@app.route('/')
async def index(req, resp):
    # Just send file
    if gc.mem_free() < 2500: # Less than ~10% RAM left
        gc.collect()
        print("Aborting serving data!")
        await resp.error(503)
        return
    await resp.send_file(path+'/static/FlightStatusBetter.html')

@app.route('/static/<fn>')
async def staticFile(req, resp, fn):
    print("Serving {}".format(path+'/static/{}'.format(fn)))
    print(get_mem_usage())
    if gc.mem_free() < 2500: # Less than ~10% RAM left
        print("Aborting serving data!")
        await resp.error(503)
        return
    await resp.send_file(path+'/static/{}'.format(fn))

@app.route('/static/controls/<fn>')
async def staticImgFile(req, resp, fn):
    print("Serving {}".format(path+'/static/controls/{}'.format(fn)))
    if gc.mem_free() < 2500: # Less than ~10% RAM left
        gc.collect()
        print("Aborting serving data!")
        await resp.error(503)
        return
    resp.add_header("Content-Encoding","gzip")
    await resp.send_file(path+'/static/controls/{}'.format(fn))

@app.route('/api/data')
async def serveStatusData(req, resp):
    """ Serve the series of changing values, comma-separated, from the data table. """
    print("Serving status data file.")
    if gc.mem_free() < 2500: # Less than ~10% RAM left
        gc.collect()
        print("Aborting serving data!")
        await resp.error(503)
        return
    await resp.send(manageData.getStatusData())

@app.route('/api/serverMessages')
async def serveServerMessages(req, resp):
    """ Serve the last 10 server messages, newline separated. """
    print("Serving server messages file.")
    outMessages = manageData.getServerMessages() # Serve the last 10 server messages
    for msg in outMessages:
        await resp.send(msg+"\n")

startTime = time.ticks_ms() / 1000
prevLoopTime = startTime
avgLoopTime = 1

print(get_mem_usage())

def mainLoop():
    # Handles loop time
    global prevLoopTime
    global avgLoopTime

    currentTime = (time.ticks_ms()/1000) - startTime
    timeDif = currentTime - prevLoopTime
    prevLoopTime = currentTime

    avgLoopTime = (timeDif+(avgLoopTime*15))/16

    ramUsage = gc.mem_alloc() / (gc.mem_alloc()+gc.mem_free()) * 100
    gc.collect()
    if gc.mem_free() < 2500: # Less than ~10% RAM left
        print("Aborting reading serial data!")
    else:
        serialdata = serial.readData()
        manageData.handleDownData(serialdata)
    gc.collect()
    manageData.handlePiLoopData(avgLoopTime,currentTime,ramUsage)

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
