import requests

# ThingsBoard credentials
url = "https://thingsboardrpi.duckdns.org"
auth = f"{url}/api/auth/login"
name = "thingsboardrpi@gmail.com"
passwd = "B87d6ol19901!"

# Auth endpoint
auth = f"{url}/api/auth/login"

while True:
    try:
        response = requests.post(auth, json={"username": name, "password": passwd})
        response.raise_for_status()  # This will raise an error for HTTP error codes
        jwt_token = response.json().get("token")

        if jwt_token:
            # Use JWT token to access telemetry endpoint
            headers = {"X-Authorization": f"Bearer {jwt_token}"}
            # Replace with your telemetry endpoint and the device ID
            telemetry_endpoint = f"{url}/api/plugins/telemetry/DEVICE/3003b0a0-6602-11ee-b68b-8d712bbc4e50/values/timeseries"
            telemetry_response = requests.get(telemetry_endpoint, headers=headers)
            telemetry_response.raise_for_status()
            print("Telemetry:", telemetry_response.json())
            break
        else:
            print("Cannot get JWT token.")

            
    except requests.RequestException as e:
        print("Error: ", e)


