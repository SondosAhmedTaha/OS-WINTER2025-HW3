import pytest
from time import sleep
from requests_futures.sessions import FuturesSession
from requests.exceptions import ConnectionError
from signal import SIGINT
from server import Server, server_port
from definitions import DYNAMIC_OUTPUT_CONTENT, SERVER_CONNECTION_OUTPUT
from utils import generate_dynamic_headers, validate_out, validate_response_full

def test_block_flush(server_port):
    with Server("./server", server_port, 3, 3, "bf") as server:
        sleep(0.1)
        session = FuturesSession()
        
        # Send 3 requests that will fill the queue
        future_a = session.get(f"http://localhost:{server_port}/output.cgi?5")
        sleep(0.1)
        future_b = session.get(f"http://localhost:{server_port}/output.cgi?5")
        sleep(0.1)
        future_c = session.get(f"http://localhost:{server_port}/output.cgi?5")
        
        sleep(0.5)  # Allow some time for requests to be processed
        
        # Send requests that should be blocked
        future_d = session.get(f"http://localhost:{server_port}/output.cgi?1")
        sleep (0.1)
        
        # only D should be dropped(explicitly closed) according to the instructions
        with pytest.raises(ConnectionError):
            future_d.result()
        sleep (0.1)
        
        future_e = session.get(f"http://localhost:{server_port}/output.cgi?1")
        
        
        # Wait for all initial requests to complete
        sleep(5.1)
        
        # Send another request that should be accepted
        future_z = session.get(f"http://localhost:{server_port}/output.cgi?1")
        
        # Verify responses
        response_a = future_a.result()
        expected_headers_a = generate_dynamic_headers(123, 1, 0, 1)
        expected_a = DYNAMIC_OUTPUT_CONTENT.format(seconds="5.0")
        validate_response_full(response_a, expected_headers_a, expected_a)
        
        response_b = future_b.result()
        expected_headers_b = generate_dynamic_headers(123, 1, 0, 1)
        expected_b = DYNAMIC_OUTPUT_CONTENT.format(seconds="5.0")
        validate_response_full(response_b, expected_headers_b, expected_b)
        
        response_c = future_c.result()
        expected_headers_c = generate_dynamic_headers(123, 1, 0, 1)
        expected_c = DYNAMIC_OUTPUT_CONTENT.format(seconds="5.0")
        validate_response_full(response_c, expected_headers_c, expected_c)
        
                
        response_z = future_z.result()
        expected_headers_z = generate_dynamic_headers(123, 2, 0, 2)
        expected_z = DYNAMIC_OUTPUT_CONTENT.format(seconds="1.0")
        validate_response_full(response_z, expected_headers_z, expected_z)
        
