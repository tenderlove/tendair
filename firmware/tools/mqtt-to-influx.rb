require "influxdb"
require "mqtt"
require "json"

influxdb = InfluxDB::Client.new "tenderhome"

MQTT::Client.connect('localhost') do |c|
  # If you pass a block to the get method, then it will loop
  c.get('/home/#') do |topic, message|
    if topic =~ /\A\/home\/([^\/]*)\/([^\/]*)\/measurement\z/
      floor = $1
      room = $2
      values = JSON.parse(message)

      # These come across as ints sometimes, but influx wants floats
      values["humidity"] = values["humidity"].to_f
      values["temperature"] = values["temperature"].to_f

      data = {
        values: values,
        tags: { floor: floor, room: room }
      }
      influxdb.write_point "measurements", data
    end
  end
end
