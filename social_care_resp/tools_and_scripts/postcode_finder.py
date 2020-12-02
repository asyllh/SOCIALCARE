# Based on: https://stackoverflow.com/questions/378157/python-regular-expression-postcode-search
import re
# from lal import address_list
def string_to_postcode(address_word):
    address = address_word.upper()
    po = re.findall(r'[A-Z]{1,2}[0-9R][0-9A-Z]?\s+[0-9][A-Z]{2}', address)
    if len(po) < 1:
        po = re.findall(r'[A-Z]{1,2}[0-9R][0-9A-Z]??[0-9][A-Z]{2}', address)

    if len(po) < 1:
        po = ["--------"]
    elif len(po) > 1:
        msg = 'WARNING: Multiple postcodes found for {0}'\
              '\n\tPresent Postcodes: {1}'\
              '\n\tPostcode added: {2}'
        print(msg.format(address, po, ))

    # Ensure there is a space in the right place:
    final_postcode = po[0]
    final_postcode = final_postcode.replace(' ', '')
    final_postcode = final_postcode[:-3] + ' ' + final_postcode[-3:]
    
    return final_postcode