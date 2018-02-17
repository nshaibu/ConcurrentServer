#!/usr/bin/python3

#===========================================================================================
# Copyright (C) 2018 Nafiu Shaibu.
# Purpose: 
#-------------------------------------------------------------------------------------------
# This is a free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by the
# Free Software Foundation; either version 3 of the License, or (at your option)
# any later version.

# This is distributed in the hopes that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
# Public License for more details.

# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

#===========================================================================================

import gi
import sys 
import signal

gi.require_version('Gtk', '3.0')
from gi.repository import Gtk, GObject 

try:
    import ipaddress as ip_object
except ImportError as err:
    print("ImportError: %s" % err.args)
    print("Install the package:[ipaddress]")
    sys.exit(1)

try:
    import pymysql
    pymysql.install_as_MySQLdb()
    import MySQLdb 
except ImportError as err:
    print("ImportError: %s\n" % err.args)
    print("Install package [pip install pymysql]")
    sys.exit(1)

import subprocess as sp

glade_file = "main_window.glade"

class server_data():

    def __init__(self):
        self.sql_server_host = "localhost"
        self.sql_server_username = None 
        self.sql_server_userpasswd = None 

        self.server_ip_addr = "127.0.0.1"
        self.server_port_num = "4040"

        self.network_config = False   #is network info configured
        self.database_config = False  #is database info configured 

    def set_sql_hostname(self, host):
        if host is None:
            self.sql_server_host = "localhost"

    def set_sql_username(self, name):
        self.sql_server_username = name

    def set_sql_userpasswd(self, passwd):
        self.sql_server_userpasswd = passwd

    def set_server_ipaddr(self, ipaddr):
        self.server_ip_addr = str(ipaddr)

    def set_server_portnum(self, port):
        self.server_port_num = str(port) 


class uptime:
    def __init__(self, widget):
        self.hours = 0
        self.mins = 0
        self.seconds = -1

        self.timout_id = None   #for the timeout_add callback function and to be able to stop the timer

        self.label_widget = widget

    def run(self, user_data):          #callback function to be run every second
        self.seconds = self.seconds + 1

        if self.seconds == 60:
            self.mins = self.mins + 1
            self.seconds = 0
        elif self.mins == 60:
            self.hours = self.hours + 1
            self.mins = 0

        time_format = '%d:%d:%d' % (self.hours, self.mins, self.seconds)
        self.label_widget.set_label(time_format)

        return True

server_info = server_data()     #global server info


class MainWindow():
    
    def __init__(self):
        #load glade file for builder
        self.builder = Gtk.Builder()
        self.builder.add_from_file(glade_file)

        #menu items
        self.about_menuitem = About_MenuItem(self.builder)
        self.about_menuitem.aboutmenu.connect("activate", self.about_menuitem.run) ##not working

        #popovers initialised
        self.startbutton_act = StartToolButton(self.builder)
        self.net_popover = NetworkPopOver(self.builder)
        self.data_popover = DatabasePopOver(self.builder)

        #get main window
        self.main_wind = self.builder.get_object("main_window")
        self.main_wind.connect("delete-event", self.on_delete)

        #get toolbar
        self.main_toolbar = self.builder.get_object("main_window_Toolbar")
        context = self.main_toolbar.get_style_context()
        context.add_class(Gtk.STYLE_CLASS_PRIMARY_TOOLBAR)

        #tool buttons
        self.start_ToolButton = self.builder.get_object("start_ToolButton")        
        self.Network_ToolButton = self.builder.get_object("Network_ToolButton")
        self.Database_ToolButton = self.builder.get_object("Database_ToolButton")

        self.start_ToolButton.connect("clicked", self.startbutton_act.run)
        self.Network_ToolButton.connect("clicked", self.net_popover.run)
        self.Database_ToolButton.connect("clicked", self.data_popover.run)

    def run(self):
        self.main_wind.show_all()
        Gtk.main()

    def on_delete(self, builder, widget):
        if self.startbutton_act.backprocess is not None:
            self.startbutton_act.backprocess.send_signal(signal.SIGINT)
            
        Gtk.main_quit()

class About_MenuItem():

    def __init__(self, builder):
        self.builder = builder 

        self.aboutmenu = self.builder.get_object("About_MenuItem")
        
        self.about_dialog = self.builder.get_object("About_dialog")
        self.about_dialog.connect("close", self.on_About_dialog_close)

    def run(self, widget):
        response = self.about_dialog.run()
        
        if response == Gtk.ResponseType.CLOSE:   
            self.about_dialog.destroy()

    def on_About_dialog_close(self, widget):
        self.about_dialog.destroy()


class StartToolButton:
    
    def __init__(self, builder):
        self.builder = builder 

        self.startbutton = self.builder.get_object("start_ToolButton")
        self.start_switch = self.builder.get_object("main_window_switch")
        self.uptime_label = self.builder.get_object("uptime_label")
        
        self.start_switch.set_tooltip_text("Server is OFF")

        self.test_serveron = False     #boolean for whether server is on or off

        self.backprocess = None    ##background process object
        self.timer = uptime(self.uptime_label)  ##timer object
        self.CMD = list()   ##background command list

        self.error_dialog = self.builder.get_object("ip_addrwrong_dialog")
                
    
    def run(self, widget):
        if self.test_serveron:    #server is on
            if self.backprocess is not None:
                self.backprocess.send_signal(signal.SIGINT)
                
                GObject.source_remove(self.timer.timout_id)  #stop uptime timer
                self.uptime_label.set_label("00:00:00")     #reset timer

                self.start_switch.set_active(False)
                self.start_switch.set_tooltip_text("Server is OFF")

            self.test_serveron = False
        else:                #server is off
            if server_info.network_config == False:
                #self.error_dialog.set_primary_text("Network not configured")
                #self.error_dialog.format_secondary_text("Give the server port and ip address to listen on")

                response = self.error_dialog.run()
                if response == Gtk.ResponseType.OK:
                    self.error_dialog.destroy()

                return
            if server_info.database_config == False:
                #self.error_dialog.set_primary_text("Database not configured")
                self.error_dialog.format_secondary_text("Configure the database to connect to it")

                response = self.error_dialog.run()
                if response == Gtk.ResponseType.OK:
                    self.error_dialog.destroy()

                return
            
            self.CMD.clear()

            self.CMD.append( '/home/nafiu/Desktop/Server/main' )   #program name
            self.CMD.append( server_info.server_port_num )
            self.CMD.append( server_info.server_ip_addr )
            self.CMD.append( server_info.sql_server_host )
            self.CMD.append( server_info.sql_server_username )
            self.CMD.append( server_info.sql_server_userpasswd )
            
            try:
                self.backprocess = sp.Popen(args=self.CMD, universal_newlines=True, stdout=sp.PIPE, stderr=sp.PIPE)
            except OSError:
                self.error_dialog.format_secondary_text("[Error]Server cannot start. Check the logs for the reason")

                response = self.error_dialog.run()
                if response == Gtk.ResponseType.OK:
                    self.error_dialog.destroy()

                return
            
            self.timer.timout_id = GObject.timeout_add(1000, self.timer.run, None)   ##set up uptime timer
            self.start_switch.set_active(True)
            self.start_switch.set_tooltip_text("Server is ON")

            self.test_serveron = True


        

class NetworkPopOver():

    def __init__(self, builder):
        self.builder = builder
        
        self.button_popover = self.builder.get_object("Network_popover")

        #text entry on the popover widget
        self.ip_textentry = self.builder.get_object("ip_addr_textentry")
        self.port_spinentry = self.builder.get_object("port_num_spinbutton")
        
        #error dialog 
        self.error_dialog = self.builder.get_object("ip_addrwrong_dialog")
        self.error_dialog.connect("close", self.on_ip_addrwrong_dialog_close)

        #set entry field values
        self.ip_textentry.set_text(server_info.server_ip_addr)
        self.port_spinentry.set_value( int(server_info.server_port_num) )

        self.save_button = self.builder.get_object("network_save_button")
        self.cancel_button = self.builder.get_object("network_cancel_button")
        
        self.save_button.connect("clicked", self.on_network_save_button_clicked)
        self.cancel_button.connect("clicked", self.on_network_cancel_button_clicked)

    def run(self, widget):
        self.button_popover.show_all()

    def on_network_save_button_clicked(self, widget):
        if self.verify_ip_addr():
            server_info.set_server_ipaddr( self.ip_textentry.get_text() )
            server_info.set_server_portnum( self.port_spinentry.get_value() )

            server_info.network_config = True

            self.button_popover.hide()
        else:
            response = self.error_dialog.run()
            
            if response == Gtk.ResponseType.OK:
                self.error_dialog.destroy()
            
    def on_ip_addrwrong_dialog_close(self, widget):
        self.error_dialog.destroy()

    def verify_ip_addr(self):
        ip = None
        
        try:
            ip = ip_object.ip_address( self.ip_textentry.get_text() )
            return ip != None
        except ValueError:
            return False

    def on_network_cancel_button_clicked(self, widget):
        self.button_popover.hide()

    def on_Network_popover_closed(self, widget):
        pass


class DatabasePopOver():

    def __init__(self, builder):
        self.builder = builder

        self.button_popover = self.builder.get_object("database_popover")

        self.error_dialog = self.builder.get_object("database_wrong_dialog")
        self.error_dialog.connect("close", self.on_database_wrong_dialog_close)

        #database entry fields
        self.sql_host_entry = self.builder.get_object("database_server_host_textentry")
        self.sql_username_entry = self.builder.get_object("sql_username_textentry")
        self.sql_passwd_entry = self.builder.get_object("sql_password_textentry")

        #set defaults and set values
        self.sql_host_entry.set_text(server_info.sql_server_host)
        
        if server_info.sql_server_username is not None:
            self.sql_username_entry.set_text(server_info.sql_server_username)
        elif server_info.sql_server_userpasswd is not None:
            self.sql_passwd_entry.set_text(server_info.sql_server_userpasswd)

        self.save_button = self.builder.get_object("database_save_button")
        self.cancel_button = self.builder.get_object("database_cancel_button")

        self.save_button.connect("clicked", self.on_database_save_button_clicked)
        self.cancel_button.connect("clicked", self.on_database_cancel_button_clicked)

    def run(self, widget):
        self.button_popover.show_all()

    def on_database_save_button_clicked(self, widget):
        if self.test_database_connection():
            server_info.set_sql_hostname( self.sql_host_entry.get_text() )
            server_info.set_sql_username( self.sql_username_entry.get_text() )
            server_info.set_sql_userpasswd( self.sql_passwd_entry.get_text() )

            server_info.database_config = True
            
            self.button_popover.hide()
        else:
            response = self.error_dialog.run()
            
            if response == Gtk.ResponseType.OK:
                self.error_dialog.destroy()

        

    def on_database_cancel_button_clicked(self, widget):
        self.button_popover.hide()

    def test_database_connection(self):
        server_host = self.sql_host_entry.get_text()
        name = self.sql_username_entry.get_text()
        passwd = self.sql_passwd_entry.get_text()

        if server_host is None:
            server_host = "localhost"
        try:
            db = MySQLdb.connect(server_host, name, passwd, "")
            
            if db is None:
                return False
        except MySQLdb.err.OperationalError:
            return False
        
        host_res_label = self.builder.get_object("host_result_label")
        server_res_label = self.builder.get_object("server_result_label")
        protocol_res_label = self.builder.get_object("protocol_result_label")
        port_res_label = self.builder.get_object("port_result_label")
        status_res_label = self.builder.get_object("status_result_label")
        cap_res_label = self.builder.get_object("cap_result_label")

        host_res_label.set_text( str(db.host) )
        server_res_label.set_text( str(db.server_version) )
        protocol_res_label.set_text( str(db.protocol_version) )
        port_res_label.set_text( str(db.port) )
        status_res_label.set_text( str(db.server_status) )
        cap_res_label.set_text( str(db.server_capabilities) )

        db.close()

        return True

    def on_database_wrong_dialog_close(self, widget):
        self.error_dialog.destroy()

    def on_database_popover_closed(self, widget):
        pass

if __name__ == '__main__':
    a = MainWindow()
    a.run()