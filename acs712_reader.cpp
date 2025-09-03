/**
 * @file acs712_reader.cpp
 * @brief Leitura de corrente utilizando o sensor ACS712 via sysfs (IIO).
 *
 * O programa lê continuamente o valor de corrente a partir do canal IIO especificado
 * e imprime no terminal a cada 100 ms.
 */

#include <iostream>
#include <fstream>
#include <string>
#include <stdexcept>
#include <thread>   ///< Necessário para std::this_thread::sleep_for
#include <chrono>   ///< Necessário para std::chrono::milliseconds

/**
 * @class ACS712
 * @brief Classe para leitura de corrente usando o sensor ACS712.
 *
 * Essa classe realiza a leitura de valores brutos do ADC via sysfs
 * e converte para corrente em ampères considerando o offset e a sensibilidade do sensor.
 */
class ACS712 {
public:
    /**
     * @brief Construtor da classe ACS712.
     * @param channel Número do canal IIO (ex.: 13).
     * @param sensitivity_mV_per_A Sensibilidade do sensor em mV/A 
     *        (ex.: 185.0 para ACS712-5A, 100.0 para ACS712-20A, 66.0 para ACS712-30A).
     */
    ACS712(int channel, float sensitivity_mV_per_A)
        : channel(channel), sensitivity(sensitivity_mV_per_A) {
        basePath = "/sys/bus/iio/devices/iio:device0/";
        rawPath = basePath + "in_voltage" + std::to_string(channel) + "_raw";
        scale = 1.0; ///< Escala fixa em mV/unidade do ADC (ajustar conforme necessário).
    }

    /**
     * @brief Lê a corrente medida pelo sensor.
     * @return Corrente em ampères.
     * @throws std::runtime_error Caso o arquivo sysfs não possa ser aberto.
     */
    float readCurrent() {
        int raw = readIntFromFile(rawPath);

        // Converte valor bruto → mV
        float voltage_mV = raw * scale;

        // Offset do sensor (meio da escala ≈ 2.5V, pode variar entre sensores)
        float zeroCurrentVoltage = 2500.0;

        // Corrente em A
        float current = (voltage_mV - zeroCurrentVoltage) / sensitivity;
        return current;
    }

private:
    int channel;              ///< Canal IIO do ADC.
    float sensitivity;        ///< Sensibilidade do sensor em mV/A.
    float scale;              ///< Fator de conversão ADC → mV.
    std::string basePath;     ///< Caminho base no sysfs.
    std::string rawPath;      ///< Caminho do arquivo raw no sysfs.

    /**
     * @brief Lê um inteiro de um arquivo sysfs.
     * @param path Caminho do arquivo sysfs.
     * @return Valor inteiro lido.
     * @throws std::runtime_error Caso o arquivo não possa ser aberto.
     */
    int readIntFromFile(const std::string &path) {
        std::ifstream file(path);
        if (!file.is_open()) {
            throw std::runtime_error("Erro ao abrir " + path);
        }
        int value;
        file >> value;
        return value;
    }
};

/**
 * @brief Função principal do programa.
 *
 * Cria uma instância do sensor ACS712 no canal 13 e imprime continuamente
 * a corrente lida a cada 100 ms.
 *
 * @return 0 se executado com sucesso, 1 em caso de erro.
 */
int main() {
    try {
        // Canal 13, sensibilidade típica 185 mV/A para ACS712 5A
        ACS712 sensor(13, 185.0);

        while (true) {
            float current = sensor.readCurrent();
            std::cout << "Corrente lida: " << current << " A" << std::endl;

            // Espera 100 ms antes da próxima leitura
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    } catch (const std::exception &e) {
        std::cerr << "Erro: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
