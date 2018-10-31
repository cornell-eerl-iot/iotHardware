
import meter_func



if __name__ == "__main__":
    try:
        while True:
            try:
                meter_func.run_meter()
            except:
                print "error!!!"
    except:
        print "undefined stop"