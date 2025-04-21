#include <iostream>
#include <string>
#include <cstdlib>
#include <sstream>
#include <cstdio>
#include <memory>
#include <stdexcept>
#include <array>
#include <regex>   

#include <iomanip>



void showMenu() {
    std::cout << "\n==== Menú de opciones ====\n";
    std::cout << "1. Obtener libro por ID\n";
    std::cout << "2. Buscar libros\n";
    std::cout << "3. Registrar usuario\n";
    std::cout << "4. Autenticar usuario\n";
    std::cout << "5. Obtener usuario\n";
    std::cout << "6. Actualizar usuario\n";
    std::cout << "7. Crear orden\n";
    std::cout << "8. Obtener orden\n";
    std::cout << "9. Listar órdenes por usuario\n";
    std::cout << "0. Salir\n";
}


std::string exec(const std::string& cmd) {
    std::array<char, 128> buffer;
    std::string result;
    std::shared_ptr<FILE> pipe(popen(cmd.c_str(), "r"), pclose);

    if (!pipe) throw std::runtime_error("popen() falló");

    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }

    return result;
}


std::string input(const std::string& prompt) {
    std::cout << prompt;
    std::string value;
    getline(std::cin, value);
    return value;
}

void runCommand(int option) {
    std::string url = "http://98.84.81.159:8081";

    switch (option) {
        case 1: {
            std::string book_id = input("📚 ID del libro: ");
            std::string cmd = "curl \"" + url + "/inventory/get?book_id=" + book_id + "\"";
            system(cmd.c_str());
            break;
        }
        case 2: {
            std::string query = input("🔍 Buscar libros por: ");
            std::string cmd = "curl \"" + url + "/inventory/search?query=" + query + "\"";
            system(cmd.c_str());
            break;
        }
	
	case 3: {
	    std::string username = input("👤 Username: ");
	    std::string email = input("📧 Email: ");
	    std::string password = input("🔐 Password: ");
	    std::string full_name = input("📝 Nombre completo: ");
	    std::string address = input("🏡 Dirección: ");

	    std::string json = "{\"username\":\"" + username + "\",\"email\":\"" + email + "\",\"password\":\"" + password +
	                       "\",\"full_name\":\"" + full_name + "\",\"address\":\"" + address + "\"}";

	    std::string cmd = "curl -s -X POST http://98.84.81.159:8081/users/register -H \"Content-Type: application/json\" -d '" + json + "'";
	    std::cout << "📡 Enviando datos...\n";

	    std::string response = exec(cmd);

	    std::cout << "📩 Respuesta: " << response << std::endl;

	    std::regex re("\"user_id\"\\s*:\\s*\"([^\"]+)\"");
	    std::smatch match;
	    if (std::regex_search(response, match, re)) {
	        std::cout << "✅ Usuario registrado con ID: " << match[1] << "\n";
	    } else {
	        std::cout << "⚠️ No se pudo obtener el ID del usuario\n";
	    }

	    break;
	}


       case 4: {
	    std::string username = input("👤 Username: ");
	    std::string password = input("🔐 Password: ");

	    std::string json = "'{\"username\":\"" + username + "\",\"password\":\"" + password + "\"}'";
	    std::string cmd = "curl -s -X POST " + url + "/users/authenticate -H \"Content-Type: application/json\" -d " + json;
	    std::string response = exec(cmd);
	    std::cout << "📩 Respuesta: " << response << std::endl;
	    break;
	}


        case 5: {
            std::string user_id = input("🆔 ID del usuario: ");
            std::string cmd = "curl \"" + url + "/users/get?user_id=" + user_id + "\"";
            system(cmd.c_str());
            break;
        }
	case 6: {
	    std::string user_id = input("🆔 ID del usuario: ");
	    std::string email = input("📧 Nuevo email: ");
	    std::string full_name = input("📝 Nuevo nombre completo: ");
	    std::string address = input("🏡 Nueva dirección: ");

	    std::string json = "'{\"user_id\":\"" + user_id + "\",\"email\":\"" + email + "\",\"full_name\":\"" + full_name + "\",\"address\":\"" + address + "\"}'";
	    std::string cmd = "curl -s -X POST " + url + "/users/update -H \"Content-Type: application/json\" -d " + json;
	    std::string response = exec(cmd);
	    std::cout << "📩 Respuesta: " << response << std::endl;
	    break;
	}

	case 7: {
	    std::string user_id = input("🆔 ID del usuario: ");
	    std::string book_id = input("📚 ID del libro: ");
	    std::string quantity = input("🔢 Cantidad: ");
	    std::string address = input("🏡 Dirección de envío: ");
	    std::string unit_price = "15.99";  // Precio por defecto

	    std::cout << "📡 Enviando datos...\n";

	    // Validar que quantity esté en formato decimal correcto
	    if (quantity.find('.') == std::string::npos) quantity += ".0";

	    std::ostringstream oss;
   	    oss << "{\"user_id\":\"" << user_id
	        << "\",\"shipping_address\":\"" << address
       	        << "\",\"items\":[{\"book_id\":\"" << book_id
	        << "\",\"quantity\":" << quantity
	        << ",\"unit_price\":" << unit_price
	        << "}]}";


	    std::string json = oss.str();

	    std::cout << "🧪 JSON que se enviará:\n" << json << "\n";

	    std::string cmd = "curl -s -X POST " + url + "/orders/create -H \"Content-Type: application/json\" -d \"" + json + "\"";
	    std::string response = exec(cmd);

	    std::cout << "📩 Respuesta: " << response << "\n";

	    // Extraer order_id
	    std::regex re("\"(order_id)\":\"([^\"]+)\"");
	    std::smatch match;
	    if (std::regex_search(response, match, re)) {
	        std::cout << "✅ Orden creada con ID: " << match[2] << "\n";
	    } else {
	        std::cout << "⚠️ No se pudo obtener el ID de la orden\n";
	    }
	    break;
	}


        case 8: {
	    std::string order_id = input("🆔 ID de la orden: ");
	    std::string cmd = "curl -s \"" + url + "/orders/get?order_id=" + order_id + "\"";

	    std::string response = exec(cmd);
	    std::cout << "📦 Orden: " << response << "\n";
	    break;
	}

	case 9: {
	    std::string user_id = input("🆔 ID del usuario: ");
	    //std::string page = input("📄 Página: ");
	    //std::string page_size = input("📑 Tamaño página: ");

	    //std::string cmd = "curl -s \"" + url + "/orders/list?user_id=" + user_id + "&page=" + 1 + "&page_size=" + 10 + "\"";
	    std::string cmd = "curl -s \"" + url + "/orders/list?user_id=" + user_id + "&page=" + std::to_string(1) + "&page_size=" + std::to_string(10) + "\"";
	    std::string response = exec(cmd);
	    std::cout << "🧾 Órdenes: \n" << response << "\n";
	    break;
	}

        case 0:
            std::cout << "👋 ¡Hasta pronto!\n";
            break;
        default:
            std::cout << "❌ Opción inválida.\n";
    }
}


int main() {
    int option;
    do {
        showMenu();
        std::cout << "\nSelecciona una opción: ";
        std::cin >> option;
        std::cin.ignore(); // limpia el buffer
        runCommand(option);
    } while (option != 0);
    return 0;
}
