# test_rack_app.rb
class TestRackApp
    def call(env)
        [200, {"Content-Type" => "text/html; charset=utf-8"},
            ["Message from Rack App."]
        ]
    end
end
