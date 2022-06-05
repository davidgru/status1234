#!/usr/bin/env python3

# Usage
#
# fetchprice {YOUR_TICKER_NAME}
# or
# python3 fetchprice {YOUR_TICKER_NAME}
# 
# Example
# fetchprice NVAX
#
# Output
# {regularMarketPrice} {regularMarketPreviousClose}
# 

import requests, sys

try:
    resp = requests.get(
        f'https://query1.finance.yahoo.com/v7/finance/quote?symbols={sys.argv[1]}',
        headers= {'User-agent': 'Mozilla/5.0'}
    )
    data = resp.json()['quoteResponse']['result'][0]
    print(data['regularMarketPrice'], data['regularMarketPreviousClose'])
except:
    print('Error')
