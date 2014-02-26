

def on_init():
    print "init!"

def on_message_receive(message):
    print "new message!", message
    return 5
