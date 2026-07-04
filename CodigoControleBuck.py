import tkinter as tk
from tkinter import ttk
import serial
import serial.tools.list_ports
import threading
import re
from collections import deque


ser = None

# Número de amostras para a média
N_MEDIA = 500

# Buffers circulares
buffer_v = deque(maxlen=N_MEDIA)
buffer_i = deque(maxlen=N_MEDIA)

#######################################################
# Conversões
#######################################################

def tensao_para_adc(v):

    valor = (v - 0.1174) / 0.0239

    valor = int(round(valor))

    valor = max(0, min(9999, valor))

    return valor



def corrente_para_adc(i):

    valor = ((i*1000) - 7.9) / 1.1651

    valor = int(round(valor))

    valor = max(0, min(9999, valor))

    return valor




def adc_para_tensao(adc):
    # Se o microcontrolador enviou 0, mostrar 0 V
    if adc == 0:
        return 0.0

    return adc * 0.0239 + 0.1174


def adc_para_corrente(adc):
    # Se o microcontrolador enviou 0, mostrar 0 A
    if adc == 0:
        return 0.0

    return (adc * 1.1651 + 7.9) / 1000



#######################################################
# UART
#######################################################

def conectar():

    global ser


    try:

        ser = serial.Serial(
                porta.get(),
                115200,
                timeout=0.1)

        status.config(text="Conectado")

        threading.Thread(
            target=receber,
            daemon=True
        ).start()


    except Exception as e:

        status.config(text=str(e))




def enviar():

    global ser

    if ser is None:
        return



    try:

        v = float(entry_v.get())
        i = float(entry_i.get())


        V = tensao_para_adc(v)
        I = corrente_para_adc(i)



        msg = f"V:{V:04d},I:{I:04d}\r\n"



        ser.write(msg.encode())


        enviado.config(
            text="TX: "+msg.strip()
        )


    except Exception as e:

        enviado.config(text=str(e))




def receber():

    global ser


    while True:

        try:

            linha = ser.readline().decode(errors='ignore')


            m = re.search(
                r'V:(\d+),I:(\d+)',
                linha
            )

            if m:


                Vadc = int(m.group(1))
                Iadc = int(m.group(2))


                Vreal = adc_para_tensao(Vadc)
                Ireal = adc_para_corrente(Iadc)

                # Adiciona ao buffer
                buffer_v.append(Vreal)
                buffer_i.append(Ireal)

                # Calcula as médias
                Vmedia = sum(buffer_v) / len(buffer_v)
                Imedia = sum(buffer_i) / len(buffer_i)

                # Exibe as médias
                recebido.config(
                    text=f"RX  V={Vmedia:.2f} V   "
                         f"I={Imedia:.3f} A"
                )
                



        except:
            pass




#######################################################
# Interface
#######################################################

janela = tk.Tk()

janela.title("Buck Terminal")
janela.geometry("420x250")



ttk.Label(janela,text="Porta COM").pack()


porta = ttk.Combobox(
            janela,
            values=[
                p.device
                for p in serial.tools.list_ports.comports()
            ]
)

porta.pack()




ttk.Button(
    janela,
    text="Conectar",
    command=conectar
).pack(pady=5)




status = ttk.Label(janela,text="Desconectado")
status.pack()



ttk.Label(janela,text="Tensão (V)")
ttk.Label(janela,text="Tensão (V)").pack()


entry_v = ttk.Entry(janela)
entry_v.insert(0,"5")
entry_v.pack()



ttk.Label(janela,text="Corrente (A)").pack()

entry_i = ttk.Entry(janela)
entry_i.insert(0,"1")
entry_i.pack()



ttk.Button(
    janela,
    text="Enviar",
    command=enviar
).pack(pady=10)



enviado = ttk.Label(janela,text="TX:")
enviado.pack()


recebido = ttk.Label(janela,text="RX:")
recebido.pack()



janela.mainloop()