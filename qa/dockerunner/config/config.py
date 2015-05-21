def load_config(path='config.yml'):
    """
    Loads the configuration file

    """
    import yaml
    
    with open(path) as config_file:
        return yaml.safe_load(config_file)
