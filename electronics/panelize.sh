rm panel/ -fr && mkdir panel/ && touch panel/OpenESPTemp-panel.kicad_pcb && cp ibom.config.ini panel/
kikit panelize -p panel_config.json OpenESPTemp.kicad_pcb panel/OpenESPTemp-panel.kicad_pcb
