import pytest
from time import sleep
from requests_futures.sessions import FuturesSession
from requests.exceptions import ConnectionError
from signal import SIGINT

from server import Server, server_port
from definitions import DYNAMIC_OUTPUT_CONTENT, SERVER_CONNECTION_OUTPUT
from utils import generate_dynamic_headers, validate_out, validate_response_full

def test_drop_head_with_skip(server_port):
    with Server("./server", server_port, 1, 3, "dh") as server:
        sleep(0.1)
        session = FuturesSession()

        # Time 0: Send requests A, B (with skip suffix), and C
        future_a = session.get(f"http://localhost:{server_port}/output.cgi?5")
        sleep(0.1)
        future_b = session.get(f"http://localhost:{server_port}/output.cgi?5.skip")
        sleep(0.1)
        future_c = session.get(f"http://localhost:{server_port}/output.cgi?5")
        
        sleep(5.2)  # Wait for A to complete and B to start

        # Time 6: Send requests D and E
        future_d = session.get(f"http://localhost:{server_port}/output.cgi?5")
        sleep(0.1)
        future_e = session.get(f"http://localhost:{server_port}/output.cgi?5")

        # Verify responses
        response_a = future_a.result()
        expected_headers_a = generate_dynamic_headers(123, 1, 0, 1)
        expected_a = DYNAMIC_OUTPUT_CONTENT.format(seconds="5.0")
        validate_response_full(response_a, expected_headers_a, expected_a)

        response_b = future_b.result()
        expected_headers_b = generate_dynamic_headers(123, 2, 0, 2)
        expected_b = DYNAMIC_OUTPUT_CONTENT.format(seconds="5.0")
        validate_response_full(response_b, expected_headers_b, expected_b)

        response_c = future_c.result()
        expected_headers_c = generate_dynamic_headers(123, 3, 0, 3)
        expected_c = DYNAMIC_OUTPUT_CONTENT.format(seconds="5.0")
        validate_response_full(response_c, expected_headers_c, expected_c)



        response_e = future_e.result()
        expected_headers_e = generate_dynamic_headers(123, 4, 0, 4)
        expected_e = DYNAMIC_OUTPUT_CONTENT.format(seconds="5.0")
        validate_response_full(response_e, expected_headers_e, expected_e)
        
                # D should be dropped
        with pytest.raises(ConnectionError):
            future_d.result()

        server.send_signal(SIGINT)
        out, err = server.communicate()
        expected_out = (
            SERVER_CONNECTION_OUTPUT.format(filename=r"/output.cgi\?5") +
            SERVER_CONNECTION_OUTPUT.format(filename=r"/output.cgi\?5") +
            SERVER_CONNECTION_OUTPUT.format(filename=r"/output.cgi\?5") +
            SERVER_CONNECTION_OUTPUT.format(filename=r"/output.cgi\?5")
        )
        validate_out(out, err, expected_out)